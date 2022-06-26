
/*
 *@@sourcefile icons.c:
 *      implementation code for object icon handling.
 *
 *      This file is ALL new with V0.9.16.
 *
 *      The WPS icon methods are quite a mess. Basically,
 *      for almost each icon method, there are two
 *      versions (query and set), and within each version,
 *      there is a "query" method and a "query data"
 *      method, respectively.
 *
 *      What these methods do isn't clearly explained
 *      anywhere. Here is what I found out, and what I
 *      think the typical flow of execution is when
 *      objects are made awake by the WPS (usually during
 *      folder populate).
 *
 *      1)  Each object stores its icon in its MINIRECORDCODE
 *          (as returned by WPObject::wpQueryCoreRecord), in
 *          the hptrIcon field. When the object is awakened,
 *          this field is initially set to NULLHANDLE.
 *
 *          MINIRECORDCORE.hptrIcon is returned by wpQueryIcon,
 *          if it is != NULLHANDLE. Otherwise wpQueryIcon does
 *          a lot of magic things depending on the object's
 *          class (see below).
 *
 *      2)  The _general_ rule of the WPS is that any object
 *          uses the object of its class, unless it has a
 *          non-default icon (indicated by the OBJSTYLE_NOTDEFAULTICON
 *          object style).
 *
 *          For most classes, the object will use the class icon.
 *          In that case, hptrIcon is set to what wpclsQueryIcon
 *          returns for the object's class. Apparently the class
 *          icon is loaded only once per class and then shared for
 *          objects that use the class icon.
 *
 *          The class icon is _not_ used however if the object
 *          has a custom icon, usually set by the user on the
 *          "Icon" page. In that case, the custom icon data is
 *          stored with the object (either in OS2.INI or in the
 *          .ICON EA), and the OBJSTYLE_NOTDEFAULTICON flag is
 *          set for the object during wpRestoreState somewhere.
 *
 *          Quite simple, and quite logical, and mostly documented.
 *
 *          The problem is of course that all subclasses of
 *          WPFileSystem (including WPFolder, WPDataFile and
 *          again WPProgramFile) plus the abstract WPProgram
 *          modify this standard behavior. And since 99% of all
 *          WPS objects are of those classes, things are different
 *          in 99% of all cases. This is where the problems
 *          start in practice when replacing the WPS standard
 *          icons.
 *
 *      3)  For data file objects, including WPProgramFile,
 *          the WPS appears to defer icon loading until the
 *          first call to wpQueryIcon, which is usually the
 *          case when the object first becomes visible in a
 *          container window. This saves the WPS from loading
 *          all icons of a folder if only a small subset of
 *          the icons actually ever becomes visible. You can
 *          notice this when scrolling thru a folder with many
 *          data file objects (especially executables).
 *
 *          So, as opposed to the standard model explained
 *          under (2), for data file objects the HPOINTER is only
 *          created during wpRestoreState if the object _does_
 *          have an .ICON EA. It is not created if the object
 *          does not have a custom icon and will later receive
 *          the icon of its associated program (or from the
 *          executable file).
 *
 *          The reason for this is speed; wpPopulate already
 *          retrieves all the EA info from disk, including the
 *          .ICON data, during DosFindFirst/Next, and this is
 *          passed to wpclsMakeAwake and then wpRestoreState in
 *          the "ulReserved" parameter that is not documented
 *          in WPSREF. Really, this is a pointer to the
 *          FILEFINDBUF3 and the FEA2LIST that was filled
 *          by wpPopulate.
 *
 *          Since the data is already present, in that case,
 *          the WPS can quickly build that data already during
 *          object instantiation.
 *
 *      4)  Now, as indicated above, the entry point into the icon mess
 *          is WPObject::wpQueryIcon. I believe this gets called by the
 *          WPS's container owner-draw routines. As a result, the method
 *          only gets called if the icon is actually visible.
 *
 *          wpQueryIcon returns the hptrIcon from the MINIRECORDCORE,
 *          if it was set already during wpRestoreState. If the field
 *          is still NULLHANDLE, the WPS _then_ builds a proper icon.
 *
 *          What happens then again depends on the object's class:
 *
 *          --  For WPDataFile, the WPS calls
 *              WPDataFile::wpSetAssociatedFileIcon to have the
 *              icon of the associated program object set for
 *              the data file too.
 *
 *          --  For WPProgramFile, WPProgramFile::wpSetProgIcon
 *              attempts to load an icon from the executable file.
 *              If that fails, a standard executable icon is used.
 *
 *          --  For WPDisk, WPDisk::wpSetCorrectDiskIcon is called
 *              to set the icon to one of the hard disk, CD-ROM
 *              or floppy icons.
 *
 *          If all of this fails, or for other classes, the WPS
 *          gets the class's default icon from wpclsQueryIcon.
 *
 *      5)  WPObject::wpSetIcon sets the MINIRECORDCORE.hptrIcon
 *          field. This can be called at any time and will also
 *          update the object's display in all containers where
 *          it is currently inserted.
 *
 *          wpSetIcon does _not_ store the icon persistently.
 *          It only changes the current icon and gets called
 *          from the various wpQueryIcon implementations when
 *          the icon is first built for an object.
 *
 *          I guess this makes it sufficiently clear that the
 *          method naming is quite silly here -- a "query" method
 *          always calls a "set" method?!? This makes it nearly
 *          impossible to override wpSetIcon, too.
 *
 *          Note that if the object had an icon previously (i.e.
 *          hptrIcon was != NULLHANDLE), the old icon is freed if
 *          the object has the OBJSTYLE_NOTDEFAULTICON flag set.
 *
 *          This flag is not changed by the method however. As
 *          a result, _after_ calling wpSetIcon, you should always
 *          call wpModifyStyle to clear or set the flag, depending
 *          on whether the icon should be freed if the object
 *          goes dormant (or will be changed again later).
 *
 *      6)  WPObject::wpQueryIconData does not return an icon,
 *          nor does it always return icon data. (Another misnamed
 *          method.) It only returns _information_ about where
 *          to find the icon; only if the object has a non-default
 *          icon, the actual icon data is returned.
 *
 *          I believe this method only gets called in some very
 *          special situations, such as on the "Icon" page and
 *          during drag'n'drop to build icon copies. Also I
 *          haven't exactly figured out how the memory management
 *          is supposed to work here.
 *
 *      7)  WPObject::wpSetIconData is supposed to change the icon
 *          permanently. Depending on the object's class, this
 *          will store the icon data in the .ICON EA or in OS2.INI.
 *          This will also call wpSetIcon in turn to change the
 *          current icon of the object.
 *
 *      Wheew. Now for the replacements. There are several problems
 *      with the WPS implementation:
 *
 *      1)  We want to replace the default icons. Unfortunately
 *          this requires us to replace the icon management
 *          altogether because for example for executables, the
 *          only public interface is WinLoadFileIcon, which
 *          _always_ builds an icon even if it's a default one.
 *
 *      2)  WinLoadFileIcon takes ages on PE executables.
 *
 *      3)  The WPS is not very good at optimizing memory usage.
 *          Basically each object has its own icon handle unless
 *          it is really the class default icon, which is quite
 *          expensive. I am now trying to at least share icons
 *          that are reused, such as our program default icons.
 *          This has turned out to be quite a mess too because
 *          the WPS sometimes frees icons where it shouldn't.
 *
 *      4)  We have to rewrite the entire icon page too, but
 *          this is a good idea in the first place because we
 *          wanted to get rid of the extra "Object" page for
 *          object hotkeys and such.
 *
 *      So we have to replace the entire executable resource
 *      management, which is in this file (icons.c).
 *      Presently NE and LX icons are supported; see icoLoadExeIcon.
 *
 *      We presently replace the following icon methods:
 *
 *      --  XWPProgram::wpQueryIcon, XWPProgram::wpSetProgIcon,
 *          XWPProgram::wpQueryIconData
 *
 *      --  XWPProgramFile::wpSetProgIcon, XWPProgramFile::wpQueryIconData
 *
 *      --  XFldDataFile::wpQueryIcon, XFldDataFile::wpQueryIconData
 *
 *      --  XWPFileSystem::wpQueryIcon, XWPFileSystem::wpQueryIconData
 *
 *      Function prefix for this file:
 *      --  ico*
 *
 *@@added V0.9.16 (2001-10-01) [umoeller]
 *@@header "filesys\icons.h"
 */

/*
 *      Copyright (C) 2001-2010 Ulrich M”ller.
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

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS

#define INCL_WINPOINTERS
#define INCL_WINSYS

#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\exeh.h"               // executable helpers
#include "helpers\standards.h"          // some standard macros
#include "helpers\winh.h"               // PM helper routines

// SOM headers which don't crash with prec. header files
// #include "xfldr.ih"

// XWorkplace implementation headers
// #include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\kernel.h"              // XWorkplace Kernel

// headers in /hook

#include "filesys\filesys.h"            // various file-system object implementation code
#include "filesys\icons.h"              // icons handling

// other SOM headers

#pragma hdrstop

/* ******************************************************************
 *
 *   Icon data handling
 *
 ********************************************************************/

typedef HPOINTER APIENTRY WINBUILDPTRHANDLE(PBYTE pbData);
typedef WINBUILDPTRHANDLE *PWINBUILDPTRHANDLE;

/*
 *@@ icoBuildPtrHandle:
 *      calls the undocumented WinBuildPtrHandle API which
 *      buils a new HPOINTER from an icon data buffer.
 *      This is faster and more convenient than
 *      WinCreatePointerIndirect where we'd have to build
 *      all the bitmaps ourselves first.
 *
 *      The API is resolved from PMMERGE.DLL on the first call.
 *
 *      An "icon data buffer" is simply the bunch of bitmap
 *      headers, color tables and bitmap data as described
 *      in PMREF; I believe it has the same format as an .ICO
 *      file and the .ICON EA. It appears that the WPS also
 *      uses this format when it stores icons in OS2.INI for
 *      abstract objects (PM_Abstract:Icons section).
 *
 *      Returns:
 *
 *      --  NO_ERROR: *phptr has received a new HPOINTER. Use
 *          WinDestroyPointer to free the icon. If you're using
 *          the icon on an object via wpSetIcon, set the
 *          OBJSTYLE_NOTDEFAULTICON style on the object.
 *
 *      --  BASEERR_BUILDPTR_FAILED: WinBuildPtrHandle failed
 *          for some unknown reason. Probably the data buffer
 *          is invalid.
 *
 *      --  ERROR_PROTECTION_VIOLATION
 *
 *      plus the return codes of doshQueryProcAddr.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.16 (2001-12-17) [lafaix]: fixed error handling and added ERROR_INVALID_HANDLE support
 */

APIRET icoBuildPtrHandle(PBYTE pbData,
                         HPOINTER *phptr)
{
    APIRET      arc = NO_ERROR;
    HPOINTER    hptr = NULLHANDLE;

    static  PWINBUILDPTRHANDLE WinBuildPtrHandle = NULL;
    BOOL    fLocked = FALSE;

    TRY_LOUD(excpt1)
    {
        if (fLocked = krnLock(__FILE__, __LINE__, __FUNCTION__))
        {
            if (!WinBuildPtrHandle)
            {
                // first call:
                if (arc = doshQueryProcAddr("PMMERGE",
                                            5117,
                                            (PFN*)&WinBuildPtrHandle))  // WinBuildPtrHandle (32-bit)
                    cmnLog(__FILE__, __LINE__, __FUNCTION__,
                           "Error %d resolving WinBuildPtrHandle from PMMERGE.",
                           arc);
            }

            krnUnlock();
            fLocked = FALSE;
        }

        if ( (!arc) && (!WinBuildPtrHandle))
            arc = ERROR_INVALID_ORDINAL;

        if (!arc)
            if (!(*phptr = WinBuildPtrHandle(pbData)))
                arc = BASEERR_BUILDPTR_FAILED;
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (fLocked)
        krnUnlock();

    return arc;
}

/*
 *@@ icoLoadICOFile:
 *      attempts to load the specified .ICO (or
 *      .PTR) file.
 *
 *      To support both "query icon" and "query
 *      icon data" interfaces, this supports three
 *      output formats:
 *
 *      --  If (phptr != NULL), the icon data
 *          is loaded and turned into a new
 *          HPOINTER by calling icoBuildPtrHandle.
 *          Use WinDestroyPointer to free that pointer.
 *
 *      --  If (pcbIconData != NULL), *pcbIconData
 *          is set to the size that is required for
 *          holding the icon data. This allows for
 *          calling the method twice to allocate
 *          a sufficient buffer dynamically.
 *
 *      --  If (pbIconData != NULL), the icon
 *          data is copied into that buffer. The
 *          caller is responsible for allocating
 *          and freeing that buffer.
 *          To properly support wpQueryIconData
 *          where it is impossible to tell the
 *          function what size the passed-in buffer
 *          is, we do not check whether the buffer
 *          is sufficiently large (V0.9.18).
 *
 *      The above is only valid if NO_ERROR is
 *      returned.
 *
 *      Otherwise this returns:
 *
 *      --  ERROR_FILE_NOT_FOUND
 *
 *      --  ERROR_PATH_NOT_FOUND
 *
 *      --  ERROR_NO_DATA: file has 0 bytes.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_INVALID_PARAMETER: pcszFilename is
 *          invalid, or pcbIconData was NULL while
 *          pbIconData was != NULL.
 *
 *      --  ERROR_BUFFER_OVERFLOW: *pcbIconData is too
 *          small a size of pbIconData. (No longer
 *          returned, V0.9.18).
 *
 *      plus the error codes of icoBuildPtrHandle.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.18 (2002-03-19) [umoeller]: no longer checking buffer size
 */

APIRET icoLoadICOFile(PCSZ pcszFilename,
                      HPOINTER *phptr,          // out: if != NULL, newly built HPOINTER
                      PULONG pcbIconData,       // out: if != NULL, size of buffer required
                      PBYTE pbIconData)         // out: if != NULL, icon data that was loaded
{
    APIRET  arc = NO_ERROR;
    PXFILE  pxf = NULL;

    PBYTE   pbData = NULL;
    ULONG   cbData = 0;

    TRY_LOUD(excpt1)
    {
        if (!(arc = doshOpen(pcszFilename,
                             XOPEN_READ_EXISTING | XOPEN_BINARY,
                             &cbData,
                             &pxf)))
        {
            if (!cbData)
                arc = ERROR_NO_DATA;
            else
            {
                if (!(pbData = malloc(cbData)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    ULONG ulDummy;
                    arc = DosRead(pxf->hf,
                                  pbData,
                                  cbData,
                                  &ulDummy);
                }
            }
        }

        // output data
        if (!arc)
        {
            // build the pointer in any case, cos
            // we can't find out whether the data
            // is broken maybe
            // V0.9.18 (2002-03-24) [umoeller]
            HPOINTER hptr;
            if (!(arc = icoBuildPtrHandle(pbData,
                                          &hptr)))
            {
                if (phptr)
                    *phptr = hptr;
                else
                    // caller does not want the pointer:
                    WinDestroyPointer(hptr);

                if (pbIconData)
                    /* if (!pcbIconData)
                        arc = ERROR_INVALID_PARAMETER;
                    else if (*pcbIconData < cbData)
                        arc = ERROR_BUFFER_OVERFLOW;
                    else */     // V0.9.18 (2002-03-19) [umoeller]
                        memcpy(pbIconData,
                               pbData,
                               cbData);

                if (pcbIconData)
                    *pcbIconData = cbData;
            }
        }
    }
    CATCH(excpt1)
    {
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (pbData)
        free(pbData);

    doshClose(&pxf);

    return arc;
}

/*
 *@@ icoBuildPtrFromFEA2List:
 *      builds a pointer from the .ICON data in
 *      the given FEA2LIST, if there's some.
 *
 *      See icoLoadICOFile for the meaning of
 *      the phptr, pcbIconData, and pbIconData
 *      parameters.
 *
 *      Returns:
 *
 *      --  NO_ERROR: output data was set. See
 *          icoLoadICOFile.
 *
 *      --  ERROR_NO_DATA: no .ICON EA found,
 *          or it has a bad format.
 *
 *      --  ERROR_INVALID_PARAMETER: pFEA2List is
 *          invalid, or pcbIconData was NULL while
 *          pbIconData was != NULL.
 *
 *      --  ERROR_BUFFER_OVERFLOW: *pcbIconData is too
 *          small a size of pbIconData. (No longer
 *          returned, V0.9.18).
 *
 *      plus the error codes of icoBuildPtrHandle.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.18 (2002-03-19) [umoeller]: no longer checking buffer size
 */

APIRET icoBuildPtrFromFEA2List(PFEA2LIST pFEA2List,     // in: FEA2LIST to check for .ICON EA
                               HPOINTER *phptr,         // out: if != NULL, newly built HPOINTER
                               PULONG pcbIconData,      // out: if != NULL, size of buffer required
                               PBYTE pbIconData)        // out: if != NULL, icon data that was loaded
{
    APIRET arc = NO_ERROR;

    if (!pFEA2List)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        PBYTE pbValue;
        if (!(pbValue = doshFindEAValue(pFEA2List,
                                        ".ICON",
                                        NULL)))
            arc = ERROR_NO_DATA;
        else
        {
            // got something:
            PUSHORT pusType = (PUSHORT)pbValue;
            if (*pusType != EAT_ICON)
                arc = ERROR_NO_DATA;
            else
            {
                // next ushort has data length
                PUSHORT pcbValue = pusType + 1;
                USHORT cbData;
                if (!(cbData = *pcbValue))
                    arc = ERROR_NO_DATA;
                else
                {
                    PBYTE pbData = (PBYTE)(pcbValue + 1);

                    // build the pointer in any case, cos
                    // we can't find out whether the data
                    // is broken maybe
                    // V0.9.18 (2002-03-24) [umoeller]
                    HPOINTER hptr;
                    if (!(arc = icoBuildPtrHandle(pbData,
                                                  &hptr)))
                    {
                        // output data
                        if (phptr)
                            *phptr = hptr;
                        else
                            WinDestroyPointer(hptr);

                        if (pbIconData)
                            /* if (!pcbIconData)
                                arc = ERROR_INVALID_PARAMETER;
                            else if (*pcbIconData < cbData)
                                arc = ERROR_BUFFER_OVERFLOW;
                            else */ // V0.9.18 (2002-03-19) [umoeller]
                                memcpy(pbIconData,
                                       pbData,
                                       cbData);

                        if (pcbIconData)
                            *pcbIconData = cbData;
                    }
                }
            }
        }
    }

    return arc;
}

/*
 *@@ icoBuildPtrFromEAs:
 *      checks if the given file has an .ICON EA
 *      and, if so, calls icoBuildPtrFromFEA2List.
 *
 *      Only if NO_ERROR is returned, the output depends
 *      on the pointers that are passed in:
 *
 *      --  If (phptr != NULL), *phptr receives the
 *          HPOINTER that was built from the .ICON EA.
 *
 *      --  If (pcbIconInfo != NULL), this puts the required size
 *          for the ICONINFO to be returned into that buffer.
 *          This is to support wpQueryIconData with a NULL
 *          pIconInfo where the required size must be returned.
 *          This will always be sizeof(ICONINFO) plus the
 *          size of the buffer for the icon data from the EA.
 *
 *      --  If (pIconInfo != NULL), this assumes that pIconInfo
 *          points to an ICONINFO structure with a sufficient
 *          size for returning the icon data.
 *          Note that we cannot check whether the buffer is large
 *          enough to hold the data because the stupid
 *          wpQueryIconData method definition has no way to tell
 *          how large the input buffer really is (since it only
 *          _returns_ the size of the data). Bad design, really.
 *          This function will always set ICONINFO.fFormat to
 *          ICON_DATA in that case, and adjust cbIconData and
 *          pIconData. The icon data is put right after the
 *          ICONINFO structure in the buffer.
 *
 *@@added V0.9.18 (2002-03-19) [umoeller]
 *@@changed V1.0.0 (2002-11-23) [umoeller]: returning HPOINTER never worked, fixed
 *@@changed V1.0.9 (2010-07-17) [pr]: added large file support @@fixes 586
 */

APIRET icoBuildPtrFromEAs(PCSZ pcszFilename,
                          HPOINTER *phptr,      // out: if != NULL, newly build icon handle
                          PULONG pcbIconInfo,   // out: if != NULL, size of ICONINFO buffer required
                          PICONINFO pIconInfo)  // out: if != NULL, icon info
{
    APIRET          arc;
    PFILEFINDBUF3L  pfb3;
    PEAOP2          peaop = NULL;

    ULONG           cbRequired = sizeof(ICONINFO);

    if (!(arc = fsysFillFindBuffer(pcszFilename,
                                   &pfb3,
                                   &peaop)))
    {
        ULONG cbData;
        PBYTE pbIconData = NULL;

        PFEA2LIST pFEA2List2 = (PFEA2LIST)(   ((PBYTE)pfb3)
                                            + FIELDOFFSET(FILEFINDBUF3L,
                                                          cchName));

        // if caller has given us a buffer already,
        // use that
        if (pIconInfo)
            pbIconData = (PBYTE)(pIconInfo + 1);

        if (!(arc = icoBuildPtrFromFEA2List(pFEA2List2,
                                            phptr, // V1.0.0 (2002-11-23) [umoeller] NULL,     // hptr
                                            &cbData,
                                            pbIconData)))
        {
            // got it:
            cbRequired += cbData;
            if (pIconInfo)
            {
                // caller wants data too:
                ZERO(pIconInfo);
                pIconInfo->cb = cbRequired;
                pIconInfo->fFormat = ICON_DATA;
                pIconInfo->cbIconData = cbData;
                pIconInfo->pIconData = pbIconData;
            }

            if (pcbIconInfo)
                *pcbIconInfo = cbRequired;
        }

        fsysFreeFindBuffer(&peaop);
    }

    return arc;
}

#pragma pack(1)
typedef struct _WIN16DIBINFO
{
    ULONG       cbFix;
    ULONG       cx;
    ULONG       cy;
    USHORT      cPlanes;
    USHORT      cBitCount;
    ULONG       cCompression;           // unused in icons
    ULONG       ulImageSize;
    ULONG       ulXPelsPerMeter;        // unused in icons
    ULONG       ulYPelsPerMeter;        // unused in icons
    ULONG       cColorsUsed;            // unused in icons
    ULONG       cColorsImportant;       // unused in icons
} WIN16DIBINFO, *PWIN16DIBINFO;
#pragma pack()

/*
 *@@ G_DefaultIconHeader:
 *      sick structure for building an icon
 *      data buffer so we can use memcpy
 *      for most fields instead of having
 *      to set everything explicitly.
 *
 *      Note that we use the old-style
 *      bitmap headers (i.e. BITMAPINFOHEADER
 *      instead of BITMAPINFOHEADER2, etc.).
 *
 *@@added V0.9.16 (2001-12-18) [umoeller]
 */

static const struct _DefaultIconHeader
{
        // bitmap array file header;
        // includes first BITMAPFILEHEADER for AND and XOR masks
        BITMAPARRAYFILEHEADER ah;
        // first RGB color table for monochrome bitmap
        RGB MonochromeTable[2];
        // second BITMAPFILEHEADER for color pointer
        BITMAPFILEHEADER fh2;
} G_DefaultIconHeader =
    {
        {
            // bitmap array file header
            /*pah->usType*/ BFT_BITMAPARRAY, //
            /*pah->cbSize*/ 40, //
            /*pah->offNext*/ 0, //       // no next array header
            /*pah->cxDisplay*/ 0, //
            /*pah->cyDisplay*/ 0, //

            // first BITMAPFILEHEADER for AND and XOR masks
            /*pah->bfh.usType*/ BFT_COLORICON, //
            /*pah->bfh.cbSize*/ 0x1A, //    // size of file header
            /*pah->bfh.xHotspot*/ 0, //         // tbr
            /*pah->bfh.yHotspot*/ 0, //     // tbr
            /* pah->bfh.offBits = */ 0, // tbr
            /*pah->bfh.bmp.cbFix*/ sizeof(BITMAPINFOHEADER), //
            /*pah->bfh.bmp.cx*/ 0, //          // tbr
            /*pah->bfh.bmp.cy*/ 0, //          // tbr
            /*pah->bfh.bmp.cPlanes*/ 1, //
            /*pah->bfh.bmp.cBitCount*/ 1, //          // monochrome
        },
        // first RGB color table for monochrome bitmap;
        // set first entry to black, second to white
        {
            0, 0, 0, 0xFF, 0xFF, 0xFF
        },
        // second BITMAPFILEHEADER for color pointer
        {
            /*pfh->usType*/ BFT_COLORICON, //
            /*pfh->cbSize*/ 0x1A, //    // size of file header
            /*pfh->xHotspot*/ 0, // tbr
            /*pfh->yHotspot*/ 0, // tbr
            /*pfh->offBits*/  0, // tbr
            /*pfh->bmp.cbFix*/ sizeof(BITMAPINFOHEADER), //
            /*pfh->bmp.cx*/ 0, // tbr
            /*pfh->bmp.cy*/ 0, // tbr
            /*pfh->bmp.cPlanes*/ 1, //
            /*pfh->bmp.cBitCount*/ 0, // tbr
        }
    };

/*
 *@@ ConvertWinIcon:
 *      converts a Windows icon to an OS/2 icon buffer.
 *
 *      At this point, we handle only 32x32 Windows icons
 *      with 16 colors.
 *
 *      As opposed to the OS/2 code, if the system icon
 *      size is 40x40, we try to center the win icon
 *      instead of scaling it (which looks quite ugly).
 *      At this point we center only vertically though
 *      because of the weird bit offsets that have to
 *      be handled.
 *
 *      Returns:
 *
 *      --  NO_ERROR: icon data was successfully converted,
 *          and *ppbResData has received the OS/2-format
 *          icon data.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_BAD_FORMAT: icon format not recognized.
 *          Currently we handle only 32x32 in 16 colors.
 *
 *@@added V0.9.16 (2001-12-18) [umoeller]
 */

STATIC APIRET ConvertWinIcon(PBYTE pbBuffer,       // in: windows icon data
                             ULONG cbBuffer,       // in: size of *pbBuffer
                             PBYTE *ppbResData,     // out: converted icon data (to be free()'d)
                             PULONG pcbResdata)   // out: size of converted data (ptr can be NULL)
{
    PWIN16DIBINFO pInfo = (PWIN16DIBINFO)pbBuffer;
    APIRET arc = ERROR_BAD_FORMAT;

    ULONG cbScanLineSrc = 0;

    ULONG cbBitmapSrc = 0;
    ULONG cbMaskLineSrc = 0;
    ULONG cbEachMaskSrc = 0;

    ULONG cColors = 1 << pInfo->cBitCount;
    ULONG cyRealSrc = pInfo->cy / 2;             // cy is doubled always

    PMPF_ICONREPLACEMENTS(("    cbFix %d", pInfo->cbFix));
    PMPF_ICONREPLACEMENTS(("    cx %d", pInfo->cx));
    PMPF_ICONREPLACEMENTS(("    cy %d", pInfo->cy));
    PMPF_ICONREPLACEMENTS(("    cPlanes %d", pInfo->cPlanes));
    PMPF_ICONREPLACEMENTS(("    cBitCount %d, cColors %d", pInfo->cBitCount, cColors));
    PMPF_ICONREPLACEMENTS(("    cColorsUsed %d", pInfo->cColorsUsed));

    /*
     * check source format:
     *
     */

    if (    (pInfo->cx == 32)
         && (cyRealSrc == 32)
         && (cColors == 16)
       )
    {
        // 32x32, 16 colors:
        cbScanLineSrc  = ((pInfo->cx + 1) / 2);    // two pixels per byte
        cbBitmapSrc    = ( cbScanLineSrc
                           * cyRealSrc
                         );         // should be 512

        cbMaskLineSrc = ((pInfo->cx + 7) / 8);      // not dword-aligned!

        cbEachMaskSrc  = cbMaskLineSrc * cyRealSrc;        // should be 128
    }

    if (cbBitmapSrc)
    {
        // after bitmap header comes color table
        // with cColors entries
        RGB2 *paColors = (RGB2*)(pbBuffer + sizeof(WIN16DIBINFO));
        ULONG cbColors = (sizeof(RGB2) * cColors);
        // next comes XOR mask in bitmap format
        // (i.e. with 16 colors, two colors in one byte)
        PBYTE pbWinXorBitmap = (PBYTE)paColors + cbColors;
        // next comes AND mask;
        // this is monochrome always
        PBYTE pbWinAndMask = (PBYTE)pbWinXorBitmap + cbBitmapSrc;

        // make sure we won't overflow
        if (pbWinAndMask + cbEachMaskSrc > pbBuffer + cbBuffer)
            arc = ERROR_BAD_FORMAT;
        else
        {
            /*
             * create OS/2 target buffer:
             *
             */

            // target icon size == system icon size (32 or 40)
            ULONG cxDest = G_cxIcon; // WinQuerySysValue(HWND_DESKTOP, SV_CXICON);
            ULONG cyRealDest = G_cyIcon; // cxDest;

            // size of one color scan line: with 16 colors,
            // that's two pixels per byte
            ULONG cbScanLineDest  = ((cxDest + 1) / 2);
            // total size of target bitmap
            ULONG cbBitmapDest    = (   cbScanLineDest
                                      * cyRealDest
                                    );         // should be 512

            // size of one monochrome scan line:
            // that's eight pixels per byte
            ULONG cbMaskLineDest2  = ((cxDest + 7) / 8);
            // note, with OS/2, this must be dword-aligned:
            ULONG cbMaskLineDest =  cbMaskLineDest2 + 3 - ((cbMaskLineDest2 + 3) & 0x03);
            // total size of a monochrome bitmap
            ULONG cbEachMaskDest  = cbMaskLineDest * cyRealDest;        // should be 128

            // total size of the OS/2 icon buffer that's needed:
            ULONG cbDataDest =   sizeof(BITMAPARRAYFILEHEADER)   // includes one BITMAPFILEHEADER
                               + 2 * sizeof(RGB)
                               + sizeof(BITMAPFILEHEADER)
                                    // the above is in G_DefaultIconHeader
                               + cColors * sizeof(RGB)   // 3
                               + 2 * cbEachMaskDest          // one XOR, one AND mask
                               + cbBitmapDest;

            // go create that buffer
            if (!(*ppbResData = malloc(cbDataDest)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                /*
                 * fill OS/2 target buffer:
                 *
                 */

                ULONG   ul;

                PBYTE   pbDest,
                        pbAndMaskDest,
                        pbXorMaskDest,
                        pbBitmapDest;

                struct _DefaultIconHeader *pHeaderDest;


                arc = NO_ERROR;

                if (pcbResdata)
                    *pcbResdata = cbDataDest;

                // start at front of buffer
                pbDest = *ppbResData;

                pHeaderDest = (PVOID)pbDest;

                // copy default header for icon from static memory
                memcpy(pHeaderDest,
                       &G_DefaultIconHeader,
                       sizeof(G_DefaultIconHeader));

                // fix the variable fields
                pHeaderDest->ah.bfh.xHotspot
                    = pHeaderDest->fh2.xHotspot
                    = cxDest / 2;
                pHeaderDest->ah.bfh.yHotspot
                    = pHeaderDest->fh2.yHotspot
                    = cyRealDest / 2;
                pHeaderDest->ah.bfh.offBits =
                              sizeof(BITMAPARRAYFILEHEADER)
                            + sizeof(RGB) * 2
                            + sizeof(BITMAPFILEHEADER)
                            + sizeof(RGB) * cColors;
                pHeaderDest->ah.bfh.bmp.cx
                    = pHeaderDest->fh2.bmp.cx
                    = cxDest;
                pHeaderDest->ah.bfh.bmp.cy = 2 * cyRealDest;         // AND and XOR masks in one bitmap

                pHeaderDest->fh2.offBits =
                              pHeaderDest->ah.bfh.offBits
                            + 2 * cbEachMaskDest;
                pHeaderDest->fh2.bmp.cy = cyRealDest;               // single size this time
                pHeaderDest->fh2.bmp.cBitCount = pInfo->cBitCount;      // same as in source

                // alright, now we have
                // -- the BITMAPARRAYFILEHEADER,
                // -- which includes the first BITMAPFILEHEADER,
                // -- the monochrome color table,
                // -- the second BITMAPFILEHEADER
                pbDest += sizeof(G_DefaultIconHeader);

                // after this comes second color table:
                // this is for the color pointer, so copy
                // from win icon, but use only three bytes
                // (Win uses four)
                for (ul = 0; ul < cColors; ul++)
                {
                    // memcpy(pbDest, &paColors[ul], 3);
                    *(PULONG)pbDest = *(PULONG)&paColors[ul];
                    pbDest += 3;
                }

                /*
                 * convert bitmap data:
                 *
                 */

                // new monochrome AND mask;
                // this we fill below... if a bit
                // is cleared here, it will be transparent
                pbAndMaskDest = pbDest;
                // clear all AND masks first; the source icon
                // might be smaller than the target icon,
                // and we want the extra space to be transparent
                memset(pbAndMaskDest, 0x00, cbEachMaskDest);
                pbDest += cbEachMaskDest;

                // new monochrome XOR mask; for this we
                // just copy the monochrome Win AND mask,
                // because with windoze, a pixel is drawn
                // only if its AND bit is set, and to draw
                // a pixel in OS/2, the XOR bit must be set too
                pbXorMaskDest = pbDest;
                memset(pbXorMaskDest, 0xFF, cbEachMaskDest);
                pbDest += cbEachMaskDest;

                // new 16-color bitmap:
                // will receive the win 16-color xor bitmap
                pbBitmapDest = pbDest;
                memset(pbBitmapDest, 0x00, cbBitmapDest);

                if (pbDest + cbBitmapDest > *ppbResData + cbDataDest)
                    arc = ERROR_BAD_FORMAT;
                else
                {
                    // line offset to convert from 32x32 to 40x40;
                    // this centers the icon vertically
                    ULONG   ulLineOfs = (cyRealDest - cyRealSrc) / 2;

                        // note that presently the below code cannot
                        // center horizontally yet because the AND
                        // and XOR masks are monochrome; with 32
                        // pixels, the AND and XOR masks take
                        // four bytes, while with 40 pixels they
                        // take five, so we'd have to distribute
                        // the four bytes onto five, which I didn't
                        // quite feel the urge for yet. ;-)

                    // set up the bunch of pointers which point
                    // to data that is needed for this scan line;
                    // all these pointers are incremented by the
                    // respective scan line size with each line
                    // loop below
                    PBYTE   pbBitmapSrcThis  =   pbWinXorBitmap;
                    PBYTE   pbBitmapDestThis =   pbBitmapDest
                                               + ulLineOfs * cbScanLineDest;

                    PBYTE   pbAndMaskSrcThis =   pbWinAndMask;
                    PBYTE   pbAndMaskDestThis =  pbAndMaskDest
                                               + ulLineOfs * cbMaskLineDest;
                    PBYTE   pbXorMaskDestThis =  pbXorMaskDest
                                               + ulLineOfs * cbMaskLineDest;

                    ULONG   x, y;

                    // run thru the source lines and set up
                    // the target lines according to the
                    // source bitmap data
                    for (y = 0;
                         y < cyRealSrc;
                         y++)
                    {
                        PBYTE   pbAndTest = pbAndMaskSrcThis,
                                pbAndSet = pbAndMaskDestThis;

                        // bit to check for AND and XOR masks;
                        // this is shifted right with every X
                        // in the loop below
                        UCHAR   ucAndMask = 0x80;     // shifted right
                        UCHAR   ucBmpMask = 0xF0;     // toggles between 0xF0 and 0x0F

                        // copy color bitmap for this line
                        memcpy(pbBitmapDestThis,
                               pbBitmapSrcThis,
                               cbScanLineSrc);

                        // copy XOR mask from Win AND mask
                        memcpy(pbXorMaskDestThis,
                               pbAndMaskSrcThis,
                               cbMaskLineSrc);

                        for (x = 0;
                             x < pInfo->cx;
                             x++)
                        {
                            // now with windoze, a pixel is drawn if its
                            // AND bit (now in the xor mask) is originally
                            // set and the pixel color is != 0; in that case,
                            // the OS/2 AND bit must be set to 1 to make
                            // it visible (non-transparent)
                            if (
                                    // check windoze AND bit; ucAndMask
                                    // is initially 0x80 and shifted right
                                    // with every X loop
                                    ((*pbAndTest) & ucAndMask)
                                    // check windoze color nibble; ucBmp
                                    // mask is initially 0xF0 and toggled
                                    // between 0x0F and 0xF0 with every
                                    // X loop
                                 && ((pbBitmapSrcThis[x / 2]) & ucBmpMask)
                               )
                            {
                                // alright, windoze pixel is visible:
                                // set the OS/2 AND bit too
                                *pbAndSet |= ucAndMask;
                            }

                            // toggle between 0xF0 and 0x0F
                            ucBmpMask ^= 0xFF;

                            // shift right the AND mask bit
                            if (!(ucAndMask >>= 1))
                            {
                                // every eight X's, reset the
                                // AND mask
                                ucAndMask = 0x80;

                                // and go for next monochrome byte
                                pbAndTest++;
                                pbAndSet++;
                            }
                        } // end for (x = 0...

                        pbBitmapSrcThis += cbScanLineSrc;
                        pbBitmapDestThis += cbScanLineDest;
                        pbAndMaskSrcThis += cbMaskLineSrc;
                        pbAndMaskDestThis += cbMaskLineDest;
                        pbXorMaskDestThis += cbMaskLineDest;
                    } // end for (y = 0;
                } // end else if (pbDest + cbBitmapDest > *ppbResData + cbDataDest)

                if (arc)
                {
                    FREE(*ppbResData);
                }
            } // end else if (!(*ppbResData = malloc(cbDataDest)))
        } // end else if (pbWinAndMask + cbEachMaskSrc > pbBuffer + cbBuffer)
    } // end if if (cbBitmapSrc)

    return arc;
}

/*
 *@@ icoGetWinResourceTypeName:
 *      returns a human-readable name for a Win
 *      resource type.
 *
 *@@added V0.9.16 (2001-12-18) [umoeller]
 *@@changed V1.0.0 (2002-09-02) [umoeller]: moved this here from program.c
 */

PSZ icoGetWinResourceTypeName(PSZ pszBuf,
                              ULONG ulTypeThis)
{
    PCSZ pcsz = "unknown";
    switch (ulTypeThis)
    {
        case WINRT_ACCELERATOR: pcsz = "WINRT_ACCELERATOR"; break;
        case WINRT_BITMAP: pcsz =  "WINRT_BITMAP"; break;
        case WINRT_CURSOR: pcsz =  "WINRT_CURSOR"; break;
        case WINRT_DIALOG: pcsz =  "WINRT_DIALOG"; break;
        case WINRT_FONT: pcsz =  "WINRT_FONT"; break;
        case WINRT_FONTDIR: pcsz =  "WINRT_FONTDIR"; break;
        case WINRT_ICON: pcsz =  "WINRT_ICON"; break;
        case WINRT_MENU: pcsz =  "WINRT_MENU"; break;
        case WINRT_RCDATA: pcsz =  "WINRT_RCDATA"; break;
        case WINRT_STRING: pcsz =  "WINRT_STRING"; break;
        case WINRT_MESSAGELIST: pcsz = "WINRT_MESSAGELIST"; break;
        case WINRT_GROUP_CURSOR: pcsz = "WINRT_GROUP_CURSOR"; break;
        case WINRT_GROUP_ICON: pcsz = "WINRT_GROUP_ICON"; break;
    }

    sprintf(pszBuf, "%d (%s)", ulTypeThis, pcsz);

    return pszBuf;
}

/*
 *@@ icoGetOS2ResourceTypeName:
 *      returns a human-readable name for an OS/2
 *      resource type.
 *
 *@@added V0.9.7 (2000-12-20) [lafaix]
 *@@changed V0.9.9 (2001-04-02) [umoeller]: now returning const char*
 *@@changed V0.9.16 (2002-01-05) [umoeller]: moved this here from fsys.c, renamed from fsysGetOS2ResourceTypeName
 *@@changed V0.9.16 (2002-01-05) [umoeller]: added icons display
 *@@changed V1.0.0 (2002-09-02) [umoeller]: moved this here from program.c
 */

PCSZ icoGetOS2ResourceTypeName(ULONG ulResourceType)
{
    switch (ulResourceType)
    {
        case RT_POINTER:
            return "Mouse pointer shape (RT_POINTER)";
        case RT_BITMAP:
            return "Bitmap (RT_BITMAP)";
        case RT_MENU:
            return "Menu template (RT_MENU)";
        case RT_DIALOG:
            return "Dialog template (RT_DIALOG)";
        case RT_STRING:
            return "String table (RT_STRING)";
        case RT_FONTDIR:
            return "Font directory (RT_FONTDIR)";
        case RT_FONT:
            return "Font (RT_FONT)";
        case RT_ACCELTABLE:
            return "Accelerator table (RT_ACCELTABLE)";
        case RT_RCDATA:
            return "Binary data (RT_RCDATA)";
        case RT_MESSAGE:
            return "Error message table (RT_MESSAGE)";
        case RT_DLGINCLUDE:
            return "Dialog include file name (RT_DLGINCLUDE)";
        case RT_VKEYTBL:
            return "Virtual key table (RT_VKEYTBL)";
        case RT_KEYTBL:
            return "Key table (RT_KEYTBL)";
        case RT_CHARTBL:
            return "Character table (RT_CHARTBL)";
        case RT_DISPLAYINFO:
            return "Display information (RT_DISPLAYINFO)";

        case RT_FKASHORT:
            return "Short-form function key area (RT_FKASHORT)";
        case RT_FKALONG:
            return "Long-form function key area (RT_FKALONG)";

        case RT_HELPTABLE:
            return "Help table (RT_HELPTABLE)";
        case RT_HELPSUBTABLE:
            return "Help subtable (RT_HELPSUBTABLE)";

        case RT_FDDIR:
            return "DBCS uniq/font driver directory (RT_FDDIR)";
        case RT_FD:
            return "DBCS uniq/font driver (RT_FD)";

        #ifndef RT_RESNAMES
            #define RT_RESNAMES         255
        #endif

        case RT_RESNAMES:
            return "String ID table (RT_RESNAMES)";
    }

    return "Application specific"; // !!! Should return value too
}

/*
 *@@ LoadWinNEResource:
 *      attempts to load the data of the resource
 *      with the specified type and id from a Win16
 *      NE executable.
 *
 *      Note that Windows uses different resource
 *      type flags than OS/2; use the WINRT_* flags
 *      from exeh.h instead of the OS/2 RT_* flags.
 *
 *      If idResource == 0, the first resource of
 *      the specified type is loaded.
 *
 *      From my testing, this code is quite efficient.
 *      Using doshReadAt, we usually get by with only
 *      two actual DosRead's from disk, including
 *      reading the actual resource data.
 *
 *      If NO_ERROR is returned, *ppbResData receives
 *      a new buffer with the resource data. The caller
 *      must free() that buffer.
 *
 *      If the resource type is WINRT_ICON, the icon
 *      data is converted to the OS/2 format, if
 *      possible. See ConvertWinIcon().
 *
 *      Otherwise this returns:
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: pExec is not NE
 *          or not Win16.
 *
 *      --  ERROR_NO_DATA: resource not found.
 *
 *      --  ERROR_BAD_FORMAT: cannot handle resource format.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

APIRET LoadWinNEResource(PEXECUTABLE pExec,     // in: executable from exehOpen
                         ULONG ulType,          // in: RT_* type (e.g. RT_POINTER)
                         ULONG idResource,      // in: resource ID or 0 for first
                         PBYTE *ppbResData,     // out: converted resource data (to be free()'d)
                         PULONG pcbResData)     // out: size of converted data (ptr can be NULL)
{
    APIRET          arc = NO_ERROR;
    ULONG           cbRead;

    ULONG           ulNewHeaderOfs = 0; // V0.9.12 (2001-05-03) [umoeller]

    PNEHEADER       pNEHeader;

    USHORT          usAlignShift;
    PXFILE          pFile;

    if (!(pNEHeader = pExec->pNEHeader))
        return ERROR_INVALID_EXE_SIGNATURE;

    if (pExec->cbDosExeHeader)
        // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
        ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

    // 1) res tbl starts with align leftshift
    pFile = pExec->pFile;
    cbRead = sizeof(usAlignShift);
    if (!(arc = doshReadAt(pFile,
                           // start of res table
                           pNEHeader->usResTblOfs
                             + ulNewHeaderOfs,
                           &cbRead,
                           (PBYTE)&usAlignShift,
                           DRFL_FAILIFLESS)))
    {
        // run thru the resources
        BOOL fPtrFound = FALSE;

        // current offset: since we want to use doshReadAt
        // for caching, we need to maintain this
        ULONG ulCurrentOfs =   pNEHeader->usResTblOfs
                             + ulNewHeaderOfs
                             + cbRead;      // should be sizeof(usAlignShift)

        while (!arc)
        {
            #pragma pack(1)
            struct WIN16_RESTYPEINFO
            {
                unsigned short  rt_id;
                unsigned short  rt_nres;
                long            reserved;
            } typeinfo;
            #pragma pack()

            //    then array of typeinfo structures, one for each res type
            //    in the file; last one has byte 0 first
            //          rtTypeID (RT_* value for this table; if 0, end of table)
            //          count of res's of this type
            //          reserved
            //          array of nameinfo structures
            //              offset rel. to beginning of file (leftshift!)
            //              length
            //              flags
            //              id (int if 0x8000 set), otherwise offset to string
            //              reserved
            //              reserved
            cbRead = sizeof(typeinfo);
            if (!(arc = doshReadAt(pFile,
                                   ulCurrentOfs,
                                   &cbRead,
                                   (PBYTE)&typeinfo,
                                   0)))
            {
                // advance our private file pointer
                ulCurrentOfs += cbRead;

                if (    (cbRead < sizeof(typeinfo))
                     || (typeinfo.rt_id == 0)
                   )
                    // this was the last array item:
                    break;
                else
                {
                    // next comes array of nameinfo structures
                    // we know how many array items follow,
                    // so we can read them in one flush, or
                    // skip them if it's not what we're looking for

                    #pragma pack(1)
                    typedef struct WIN16_RESNAMEINFO
                    {
                        // The following two fields must be shifted left by the value of
                        // the rs_align field to compute their actual value.  This allows
                        // resources to be larger than 64k, but they do not need to be
                        // aligned on 512 byte boundaries, the way segments are
                        unsigned short rn_offset;   // file offset to resource data
                        unsigned short rn_length;   // length of resource data
                        unsigned short rn_flags;    // resource flags
                        unsigned short rn_id;       // resource name id
                        unsigned short rn_handle;   // If loaded, then global handle
                        unsigned short rn_usage;    // Initially zero.  Number of times
                                                    // the handle for this resource has
                                                    // been given out
                    } nameinfo;
                    #pragma pack()

                    ULONG ulTypeThis = typeinfo.rt_id & ~0x8000;

                    if (    (!(typeinfo.rt_id & 0x8000))
                         || (ulTypeThis != ulType)
                       )
                    {
                        // this is not our type, so we can simply
                        // skip the entire table for speed

                        #ifdef __DEBUG__
                            CHAR szBuf[100];
                            PMPF_ICONREPLACEMENTS(("skipping type %d (%s), %d entries",
                                          ulTypeThis,
                                          icoGetWinResourceTypeName(szBuf, ulTypeThis),
                                          typeinfo.rt_nres));
                        #endif

                        ulCurrentOfs += typeinfo.rt_nres * sizeof(nameinfo);
                    }
                    else
                    {
                        // this is our type:
                        nameinfo *paNameInfos = NULL;
                        ULONG cbNameInfos;

                        #ifdef __DEBUG__
                            CHAR szBuf[100];
                            PMPF_ICONREPLACEMENTS(("entering type %d (%s), %d entries",
                                          ulTypeThis,
                                          icoGetWinResourceTypeName(szBuf, ulTypeThis),
                                          typeinfo.rt_nres));
                        #endif

                        if (    (!(arc = doshAllocArray(typeinfo.rt_nres,
                                                        sizeof(nameinfo),
                                                        (PBYTE*)&paNameInfos,
                                                        &cbNameInfos)))
                             && (cbRead = cbNameInfos)
                             && (!(arc = doshReadAt(pFile,
                                                    ulCurrentOfs,
                                                    &cbRead,
                                                    (PBYTE)paNameInfos,
                                                    DRFL_FAILIFLESS)))
                           )
                        {
                            ULONG ul;
                            nameinfo *pThis = paNameInfos;

                            ulCurrentOfs += cbRead;

                            for (ul = 0;
                                 ul < typeinfo.rt_nres;
                                 ul++, pThis++)
                            {
                                ULONG ulIDThis = pThis->rn_id;
                                ULONG ulOffset = pThis->rn_offset << usAlignShift;
                                ULONG cbThis =   pThis->rn_length << usAlignShift;

                                PMPF_ICONREPLACEMENTS(("   found res type %d, id %d, length %d",
                                            ulTypeThis,
                                            ulIDThis & ~0x8000,
                                            cbThis));

                                if (    (!idResource)
                                     || (    (ulIDThis & 0x8000)
                                          && ((ulIDThis & ~0x8000) == idResource)
                                        )
                                   )
                                {
                                    // found:
                                    PBYTE pb;
                                    if (!(pb = malloc(cbThis)))
                                        arc = ERROR_NOT_ENOUGH_MEMORY;
                                    else
                                    {
                                        if (!(arc = doshReadAt(pFile,
                                                               ulOffset,
                                                               &cbThis,
                                                               pb,
                                                               DRFL_FAILIFLESS)))
                                        {
                                            if (ulType == WINRT_ICON)
                                            {
                                                ULONG cbConverted = 0;
                                                if (!ConvertWinIcon(pb,
                                                                    cbThis,
                                                                    ppbResData,
                                                                    &cbConverted))
                                                {
                                                    if (pcbResData)
                                                        *pcbResData = cbConverted;
                                                    fPtrFound = TRUE;
                                                }
                                                // else unknown format: keep looking

                                                // but always free the buffer,
                                                // since ConvertWinIcon has created one
                                                free(pb);
                                            }
                                            else
                                            {
                                                // not icon: just return this
                                                *ppbResData = pb;
                                                if (pcbResData)
                                                    *pcbResData = cbThis;
                                                fPtrFound = TRUE;
                                            }
                                        }
                                        else
                                        {
                                            free(pb);
                                            // stop reading, we have a problem
                                            break;
                                        }
                                    }
                                }

                                if (fPtrFound || arc)
                                    break;

                            } // end for
                        }

                        if (paNameInfos)
                            free(paNameInfos);

                        // since this was the type we were looking
                        // for, stop in any case
                        break;  // while

                    } // end if our type
                }
            } // end if (!(arc = DosRead(pExec->hfExe,
        } // end while (!arc)

        if ((!fPtrFound) && (!arc))
            arc = ERROR_NO_DATA;
    }

    return arc;
}

typedef unsigned short  WORD;
typedef unsigned long   DWORD;

typedef CHAR           *LPSTR;
typedef const CHAR     *LPCSTR;

// typedef unsigned short  WCHAR;
typedef WCHAR          *LPWSTR;
typedef WCHAR          *PWSTR;
typedef const WCHAR    *LPCWSTR;
typedef const WCHAR    *PCWSTR;

// #define LOBYTE(w)              ((BYTE)(WORD)(w))
// #define HIBYTE(w)              ((BYTE)((WORD)(w) >> 8))

#define LOWORD(l)              ((WORD)(DWORD)(l))
#define HIWORD(l)              ((WORD)((DWORD)(l) >> 16))

#define SLOWORD(l)             ((INT16)(LONG)(l))
#define SHIWORD(l)             ((INT16)((LONG)(l) >> 16))

// #define MAKEWORD(low,high)     ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
// #define MAKELONG(low,high)     ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
// #define MAKELPARAM(low,high)   ((LPARAM)MAKELONG(low,high))
// #define MAKEWPARAM(low,high)   ((WPARAM)MAKELONG(low,high))
// #define MAKELRESULT(low,high)  ((LRESULT)MAKELONG(low,high))
// #define MAKEINTATOM(atom)      ((LPCSTR)MAKELONG((atom),0))

/*
 *@@ GetResDirEntryW:
 *      helper function, goes down one level of PE resource
 *      tree.
 *
 *@@added V0.9.16 (2002-01-09) [umoeller]
 */

PIMAGE_RESOURCE_DIRECTORY GetResDirEntryW(PIMAGE_RESOURCE_DIRECTORY resdirptr,
                                          LPCWSTR name,
                                          ULONG root,
                                          BOOL allowdefault)
{
    int entrynum;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY entryTable;
    int namelen;

    if (HIWORD(name))
    {
        // to make this work we'd have to copy the entire
        // Odin codepage management which I do _not_ want
        /*
        if (name[0] == '#')
        {
            char    buf[10];

            lstrcpynWtoA(buf,
                         name + 1,
                         10);
            return GetResDirEntryW(resdirptr,
                                   (LPCWSTR)atoi(buf),
                                   root,
                                   allowdefault);
        }
        entryTable = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(   (BYTE*)resdirptr
                                                        + sizeof(IMAGE_RESOURCE_DIRECTORY));
        namelen = lstrlenW(name);
        for (entrynum = 0;
             entrynum < resdirptr->NumberOfNamedEntries;
             entrynum++)
        {
            PIMAGE_RESOURCE_DIR_STRING_U str
                = (PIMAGE_RESOURCE_DIR_STRING_U)(   root
                                                  + entryTable[entrynum].u1.s.NameOffset);
            if (namelen != str->Length)
                continue;
            if (lstrncmpiW(name,
                           str->NameString,
                           str->Length)==0)
                    return (PIMAGE_RESOURCE_DIRECTORY)(   root
                                                        + entryTable[entrynum].u2.s.OffsetToDirectory);
        }
        */
    }
    else
    {
        entryTable
            = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)
                    (
                        (BYTE*)resdirptr
                      + sizeof(IMAGE_RESOURCE_DIRECTORY)
                      +   resdirptr->NumberOfNamedEntries
                        * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY)
                    );
        for (entrynum = 0;
             entrynum < resdirptr->NumberOfIdEntries;
             entrynum++)
        {
            if ((DWORD)entryTable[entrynum].u1.Name == (DWORD)name)
                return (PIMAGE_RESOURCE_DIRECTORY)(   root
                                                    + entryTable[entrynum].u2.s.OffsetToDirectory);
        }
        // just use first entry if no default can be found
        if (allowdefault && !name && resdirptr->NumberOfIdEntries)
            return (PIMAGE_RESOURCE_DIRECTORY)(   root
                                                + entryTable[0].u2.s.OffsetToDirectory);
    }

    return NULL;
}

/*
 *@@ LoadRootResDirectory:
 *
 *@@added V0.9.16 (2002-01-09) [umoeller]
 */

APIRET LoadRootResDirectory(PEXECUTABLE pExec,
                            PIMAGE_SECTION_HEADER paSections,
                            PIMAGE_RESOURCE_DIRECTORY *ppResDir,    // out: new directory
                            PULONG pcbResDir)           // out: size
{
    APIRET arc = ERROR_NO_DATA;     // unless found

    PPEHEADER pPEHeader = pExec->pPEHeader;

    // address to find is address specified in resource data directory
    ULONG ulAddressFind = pPEHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

    int i;

    PMPF_ICONREPLACEMENTS(("entering, %d sections, looking for 0x%lX",
                pPEHeader->FileHeader.usNumberOfSections,
                ulAddressFind));

    for (i = 0;
         i < pPEHeader->FileHeader.usNumberOfSections;
         i++)
    {
        PIMAGE_SECTION_HEADER pThis = &paSections[i];

        PMPF_ICONREPLACEMENTS(("    %d (%s): virtual address 0x%lX",
                i,
                pThis->Name,
                pThis->VirtualAddress));

        if (pThis->flCharacteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
            continue;

        // FIXME: doesn't work when the resources are not in a seperate section
        if (pThis->VirtualAddress == ulAddressFind)
        {
            // rootresdir = (PIMAGE_RESOURCE_DIRECTORY)(   (char*)pExec->pDosExeHeader
               //                                        + pThis->ulPointerToRawData);

            ULONG cb = pThis->ulSizeOfRawData; // sizeof(IMAGE_RESOURCE_DIRECTORY);
            PIMAGE_RESOURCE_DIRECTORY pResDir;

            PMPF_ICONREPLACEMENTS(("raw data size %d, ptr %d",
                    pThis->ulSizeOfRawData,
                    pThis->ulPointerToRawData,
                    sizeof(IMAGE_RESOURCE_DIRECTORY)));

            if (!(pResDir = malloc(cb)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                // load that
                if (!(arc = doshReadAt(pExec->pFile,
                                       // offset:
                                       pThis->ulPointerToRawData,
                                       &cb,
                                       (PBYTE)pResDir,
                                       DRFL_FAILIFLESS)))
                {
                    *ppResDir = pResDir;
                    *pcbResDir = cb;
                }
                else
                    free(pResDir);
            }

            break;
        }
    }

    PMPF_ICONREPLACEMENTS(("returning %d", arc));

    return arc;
}

#define MAKEINTRESOURCEW(i) (LPWSTR)((DWORD)((WORD)(i)))
#define RT_ICONW            MAKEINTRESOURCEW(3)
#define RT_GROUP_ICONW      MAKEINTRESOURCEW(14)

/*
 *@@ LoadResData:
 *
 *@@added V0.9.16 (2002-01-09) [umoeller]
 */

APIRET LoadResData(PPEHEADER pPEHeader,
                   PIMAGE_SECTION_HEADER paSections,
                   PIMAGE_RESOURCE_DIRECTORY pRootResDir,
                   PIMAGE_RESOURCE_DIRECTORY icongroupresdir,
                   int iInIconDir,               // current index
                   int iconDirCount,
                   int idResource)
{
    APIRET          arc = NO_ERROR;

    /* PBYTE           idata,
                    igdata; */

    return arc;
}

/*
 *@@ LoadWinPEResource:
 *
 *@@added V0.9.16 (2002-01-09) [umoeller]
 */

APIRET LoadWinPEResource(PEXECUTABLE pExec,     // in: executable from exehOpen
                         ULONG ulType,          // in: RT_* type (e.g. RT_POINTER)
                         int idResource,      // in: resource ID or 0 for first
                         PBYTE *ppbResData,     // out: converted resource data (to be free()'d)
                         PULONG pcbResData)     // out: size of converted data (ptr can be NULL)
{
    APIRET          arc = NO_ERROR;
    ULONG           cbRead;

    ULONG           ulNewHeaderOfs = 0; // V0.9.12 (2001-05-03) [umoeller]

    PPEHEADER       pPEHeader;

    PXFILE          pFile;

    PIMAGE_SECTION_HEADER
                    paSections = NULL;
    ULONG           cbSections;

    PIMAGE_RESOURCE_DIRECTORY
                    pRootResDir = NULL;
    ULONG           cbRootResDir;
    PIMAGE_RESOURCE_DIRECTORY
                    icongroupresdir = NULL;

    if (!(pPEHeader = pExec->pPEHeader))
        return ERROR_INVALID_EXE_SIGNATURE;
    if (    (pExec->ulOS != EXEOS_WIN32_GUI)
         && (pExec->ulOS != EXEOS_WIN32_CLI)
       )
        return ERROR_INVALID_EXE_SIGNATURE;

    if (pExec->cbDosExeHeader)
        // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
        ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

    pFile = pExec->pFile;

    // read in section headers right after PE header
    if (    (!(arc = doshAllocArray(pPEHeader->FileHeader.usNumberOfSections,
                                    sizeof(IMAGE_SECTION_HEADER),
                                    (PBYTE*)&paSections,
                                    &cbSections)))
         && (!(arc = doshReadAt(pFile,
                                // right after PE header
                                ulNewHeaderOfs + sizeof(PEHEADER), // pExec->cbPEHeader,
                                &cbSections,
                                (PBYTE)paSections,
                                DRFL_FAILIFLESS)))
        // paSections = (PIMAGE_SECTION_HEADER)(   ((char*)pPEHeader)
           //                             + sizeof(*pPEHeader));
                              // probably makes problems with short PE headers...
         && (!(arc = LoadRootResDirectory(pExec,
                                          paSections,
                                          &pRootResDir,
                                          &cbRootResDir)))
       )
    {
        // search the group icon dir
        if (!(icongroupresdir = GetResDirEntryW(pRootResDir,
                                                RT_GROUP_ICONW,
                                                (ULONG)pRootResDir,
                                                FALSE)))
            arc = ERROR_NO_DATA; // WARN("No Icongroupresourcedirectory!\n");
        else
        {
            // alright, found the icon group dir:
            ULONG iconDirCount =   icongroupresdir->NumberOfNamedEntries
                                 + icongroupresdir->NumberOfIdEntries;

            int     ulIconGroup = 0;        // n

            // res dir for icon groups follows
            PIMAGE_RESOURCE_DIRECTORY_ENTRY
                    pResDirEntryThis = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(icongroupresdir + 1);

            BOOL fPtrFound = FALSE;

            PMPF_ICONREPLACEMENTS(("  found RT_GROUP_ICONW, %d names, %d ids",
                    icongroupresdir->NumberOfNamedEntries,
                    icongroupresdir->NumberOfIdEntries));

            // go thru icon group
            for (ulIconGroup = 0;
                 ((ulIconGroup < iconDirCount) && (pResDirEntryThis)) && (!arc);
                 ulIconGroup++, pResDirEntryThis++)
            {
                PMPF_ICONREPLACEMENTS(("  %d: idThis: %d, ofs to data 0x%lX",
                        ulIconGroup,
                        pResDirEntryThis->u1.Id,
                        pResDirEntryThis->u2.OffsetToData));

                if (    (idResource == 0)       // first one found
                     || (pResDirEntryThis->u1.Id == idResource)
                   )
                {
                    if (ulIconGroup >= iconDirCount) // idResource %d is larger than iconDirCount %d\n",idResource,iconDirCount);
                        arc = ERROR_NO_DATA;
                    else
                    {
                        // found icon:
                        PIMAGE_RESOURCE_DATA_ENTRY
                                        idataent,
                                        igdataent;
                        PIMAGE_RESOURCE_DIRECTORY_ENTRY
                                        pXResDirEntry;
                        int             i,
                                        j;

                        pXResDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(icongroupresdir + 1);

                        if (ulIconGroup <= iconDirCount - idResource)   // assure we don't get too much ...
                        {
                            pXResDirEntry = pXResDirEntry + idResource;     // starting from specified index ...

                            PMPF_ICONREPLACEMENTS(("    found icondir for id %d", pResDirEntryThis->u1.Id));

                            for (i = 0;
                                 i < ulIconGroup;
                                 i++, pXResDirEntry++)
                            {
                                PIMAGE_RESOURCE_DIRECTORY   resdir;

                                // go down this resource entry, name
                                resdir = (PIMAGE_RESOURCE_DIRECTORY)(  (ULONG)pRootResDir
                                                                      +(pXResDirEntry->u2.s.OffsetToDirectory));

                                // default language (0)
                                resdir = GetResDirEntryW(resdir,
                                                         0,
                                                         (ULONG)pRootResDir,
                                                         TRUE);
                                igdataent = (PIMAGE_RESOURCE_DATA_ENTRY)resdir;

                                // lookup address in mapped image for virtual address
                                // igdata = NULL;

                                for (j = 0;
                                     j < pPEHeader->FileHeader.usNumberOfSections;
                                     j++)
                                {
                                    ULONG ulOfs;
                                    PIMAGE_RESOURCE_DIRECTORY iconresdir = NULL;

                                    if (igdataent->OffsetToData < paSections[j].VirtualAddress)
                                        continue;
                                    if (   igdataent->OffsetToData + igdataent->Size
                                         >   paSections[j].VirtualAddress
                                           + paSections[j].ulSizeOfRawData)
                                        continue;

                                    ulOfs = //  (PBYTE)pExec->pDosExeHeader +
                                            (   igdataent->OffsetToData
                                              - paSections[j].VirtualAddress
                                              + paSections[j].ulPointerToRawData
                                            );

                                    PMPF_ICONREPLACEMENTS(("    data of this icon group is at 0x%lX", ulOfs));

                                    /* RetPtr[i] = (HICON)pLookupIconIdFromDirectoryEx(igdata,
                                                                                    TRUE,
                                                                                    cxDesired,
                                                                                    cyDesired,
                                                                                    LR_DEFAULTCOLOR);
                                    */

                                    /*
                                    if (iconresdir = GetResDirEntryW(pRootResDir,
                                                                     RT_ICONW,
                                                                     (ULONG)pRootResDir,
                                                                     FALSE))
                                    {
                                        for (i2 = 0;
                                             (i2 < ulIconGroup) && (!arc);
                                             i2++)
                                        {
                                            PIMAGE_RESOURCE_DIRECTORY   xresdir;
                                            xresdir = GetResDirEntryW(iconresdir,
                                                                      (LPWSTR)(ULONG)RetPtr[i2],
                                                                      (ULONG)pRootResDir,
                                                                      FALSE);
                                            xresdir = GetResDirEntryW(xresdir,
                                                                      (LPWSTR)0,
                                                                      (ULONG)pRootResDir,
                                                                      TRUE);
                                            idataent = (PIMAGE_RESOURCE_DATA_ENTRY)xresdir;

                                            // map virtual to address in image
                                            for (j2 = 0;
                                                 j2 < pPEHeader->FileHeader.usNumberOfSections;
                                                 j2++)
                                            {
                                                ULONG ulDataOfs;
                                                if (idataent->OffsetToData < paSections[j].VirtualAddress)
                                                    continue;
                                                if (idataent->OffsetToData+idataent->Size > paSections[j].VirtualAddress+paSections[j].SizeOfRawData)
                                                    continue;
                                                ulDataOfs =
                                                          (   idataent->OffsetToData
                                                            - paSections[j].VirtualAddress
                                                            + paSections[j].ulPointerToRawData
                                                          );
                                                RetPtr[i] = (HICON)pCreateIconFromResourceEx(idata,
                                                                                             idataent->Size,
                                                                                             TRUE,
                                                                                             0x00030000,
                                                                                             cxDesired,
                                                                                             cyDesired,
                                                                                             LR_DEFAULTCOLOR);
                                            }
                                        } // for i = 0
                                    }

                                    hRet = RetPtr[0]; // return first icon
                                    break;
                                    */

                                    break;
                                }
                            }
                        }
                    } // else if (n >= iconDirCount)

                    if (fPtrFound)
                        break;

                } // if idResource

            } // while
        }

        if (paSections)
            free(paSections);
        if (pRootResDir)
            free(pRootResDir);
    }

    arc = ERROR_NO_DATA;

    return arc;
}

/*
 *@@ icoLoadExeIcon:
 *      smarter replacement for WinLoadFileIcon.
 *      In conjunction with the exeh* functions,
 *      this is a full rewrite. This does not call
 *      the OS/2 loader, but completely parses the
 *      executable structures itself at ring 3.
 *
 *      Essentially, this function allows you to
 *      get any icon resource from an executable
 *      file without having to run DosLoadModule.
 *      This is used in XWorkplace for getting the
 *      icons for executable files.
 *
 *      Differences to WinLoadFileIcon:
 *
 *      --  WinLoadFileIcon can take ages on PE files.
 *
 *      --  WinLoadFileIcon _always_ returns an icon,
 *          which makes it unsuitable for our
 *          wpSetProgIcon replacements because we'd
 *          rather replace the default icons and we
 *          can't find out if there's a non-default
 *          icon using WinLoadFileIcon.
 *
 *      --  This is intended for executables _only_.
 *          It does not check for an .ICO file in the
 *          same directory, nor will this check for
 *          .ICON EAs, as WinLoadFileIcon would.
 *
 *      --  This takes an EXECUTABLE from exehOpen
 *          as input. As a result, only executables
 *          supported by exehOpen are supported.
 *          (But that's NE, LX, and PE anyway.)
 *
 *      Presently the following executable and icon
 *      resource formats are understood:
 *
 *      1)  OS/2 LX and NE (via exehLoadLXResource
 *          and exehLoadOS2NEResource);
 *
 *      2)  Win16 NE, but only 32x32 icons in 16 colors.
 *
 *      PE icons are still not working.
 *
 *      See icoLoadICOFile for the meaning of
 *      the phptr, pcbIconData, and pbIconData
 *      parameters.
 *
 *      This returns:
 *
 *      --  NO_ERROR: output data was set. See
 *          icoLoadICOFile.
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: cannot handle
 *          this EXE format.
 *
 *      --  ERROR_NO_DATA: EXE format understood, but
 *          the EXE contains no icon resources.
 *          Still returned for PE now.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_INVALID_PARAMETER: pExec is
 *          invalid, or pcbIconData was NULL while
 *          pbIconData was != NULL.
 *
 *      --  ERROR_BUFFER_OVERFLOW: *pcbIconData is too
 *          small a size of pbIconData. (No longer
 *          returned, V0.9.18.)
 *
 *      plus the error codes of exehOpen and icoBuildPtrHandle.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.18 (2002-03-19) [umoeller]: no longer checking buffer size
 *@@changed V0.9.20 (2002-07-03) [umoeller]: fixed major screwup if pExec was NULL
 *@@changed V1.0.2 (2003-11-13) [umoeller]: adjusted for new LX func prototype
 */

APIRET icoLoadExeIcon(PEXECUTABLE pExec,        // in: EXECUTABLE from exehOpen
                      ULONG idResource,         // in: resource ID or 0 for first
                      HPOINTER *phptr,          // out: if != NULL, newly built HPOINTER
                      PULONG pcbIconData,       // out: if != NULL, size of buffer required
                      PBYTE pbIconData)         // out: if != NULL, icon data that was loaded
{
    APIRET  arc;
    PBYTE   pbDataFree = NULL,
            pbDataUse = NULL;
    ULONG   cbData = 0,
            ulOfs;

    static  s_fCrashed = FALSE;

    if (s_fCrashed)
        return ERROR_PROTECTION_VIOLATION;

    // moved this out of the excpt handler, christ
    // V0.9.20 (2002-07-03) [umoeller]
    if (!pExec)
        return ERROR_INVALID_PARAMETER;

    TRY_LOUD(excpt1)
    {
        // check the executable type
        switch (pExec->ulExeFormat)
        {
            case EXEFORMAT_LX:
                // these two we can handle for now
                if (!(arc = exehLoadLXResource(pExec,
                                               RT_POINTER,
                                               idResource,
                                               &pbDataFree,
                                               &ulOfs,
                                               &cbData)))
                    pbDataUse = pbDataFree + ulOfs;
            break;

            case EXEFORMAT_NE:
                switch (pExec->ulOS)
                {
                    case EXEOS_OS2:
                        arc = exehLoadOS2NEResource(pExec,
                                                    RT_POINTER,
                                                    idResource,
                                                    &pbDataFree,
                                                    &cbData);
                        #ifdef __DEBUG__
                            if (arc)
                                PMPF_ICONREPLACEMENTS(("LoadOS2NEResource returned %d", arc));
                        #endif
                    break;

                    case EXEOS_WIN16:
                    case EXEOS_WIN386:
                        arc = LoadWinNEResource(pExec,
                                                WINRT_ICON,
                                                idResource,
                                                &pbDataFree,
                                                &cbData);
                        #ifdef __DEBUG__
                            if (arc)
                                PMPF_ICONREPLACEMENTS(("LoadWinNEResource returned %d", arc));
                        #endif
                    break;

                    default:
                        arc = ERROR_INVALID_EXE_SIGNATURE;
                }
            break;

            case EXEFORMAT_PE:
                arc = LoadWinPEResource(pExec,
                                        WINRT_ICON,
                                        idResource,
                                        &pbDataFree,
                                        &cbData);

                PMPF_ICONREPLACEMENTS(("LoadWinPEResource returned %d", arc));
            break;

            default:        // includes COM, BAT, CMD
                arc = ERROR_INVALID_EXE_SIGNATURE;
            break;
        }

        // output data
        if (!arc)
        {
            if (!pbDataUse && pbDataFree)
                pbDataUse = pbDataFree;

            if (pbDataUse && cbData)
            {
                // build the pointer in any case, cos
                // we can't find out whether the data
                // is broken maybe
                // V0.9.18 (2002-03-24) [umoeller]
                HPOINTER hptr;
                if (!(arc = icoBuildPtrHandle(pbDataUse,
                                              &hptr)))
                {
                    if (phptr)
                        *phptr = hptr;
                    else
                        WinDestroyPointer(hptr);

                    if (pbIconData)
                        /* if (!pcbIconData)
                            arc = ERROR_INVALID_PARAMETER;
                        else if (*pcbIconData < cbData)
                            arc = ERROR_BUFFER_OVERFLOW;
                        else */ // V0.9.18 (2002-03-19) [umoeller]
                            memcpy(pbIconData,
                                   pbDataUse,
                                   cbData);

                    if (pcbIconData)
                        *pcbIconData = cbData;
                }
            }
            else
                arc = ERROR_NO_DATA;
        }
    }
    CATCH(excpt1)
    {
        s_fCrashed = TRUE;      // never let this code do anything again
        arc = ERROR_PROTECTION_VIOLATION;
    } END_CATCH();

    if (pbDataFree)
        free(pbDataFree);

    #ifdef __DEBUG__
    if (arc)
        PMPF_ICONREPLACEMENTS(("returning %d", arc));
    #endif

    return arc;
}


