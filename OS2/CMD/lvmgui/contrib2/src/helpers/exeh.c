
/*
 *@@sourcefile exeh.c:
 *      contains code to load and parse executable headers
 *      and resources. See exehOpen for details.
 *
 *      This file is new with V0.9.16 (2002-01-05) [umoeller]
 *      and contains code formerly in dosh2.c.
 *
 *      Function prefixes:
 *      --  exe*   executable helper functions.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\exeh.h"
 */

/*
 *      This file Copyright (C) 2000-2002 Ulrich M”ller,
 *                                        Martin Lafaix.
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

#define INCL_DOSPROFILE
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSESMGR
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_WINPROGRAMLIST
#include <os2.h>

// #include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/dosh.h"
#include "helpers/ensure.h"
#include "helpers/except.h"
#include "helpers/exeh.h"
#include "helpers/standards.h"
#include "helpers/stringh.h"

#pragma hdrstop

/*
 *@@category: Helpers\Control program helpers\Executable info
 *      these functions can retrieve BLDLEVEL information,
 *      imported modules information, exported functions information,
 *      and resources information from any executable module. See
 *      exehOpen.
 */

/********************************************************************
 *
 *   Executable functions
 *
 ********************************************************************/

/*
 *@@ exehOpen:
 *      this opens the specified executable file
 *      (which can be an .EXE, .COM, .DLL, or
 *      driver file) for use with the other
 *      exeh* functions.
 *
 *      Basically this does a plain DosOpen on the
 *      executable file and reads in the various
 *      executable headers manually. Since DosLoadModule
 *      et al is never used, the OS/2 executable loader
 *      is completely circumvented. (Side note:
 *      DosLoadModule cannot be used on EXE files in
 *      the first place.)
 *
 *      To be more precise, this uses doshOpen internally
 *      and can thus profit from the caching that is
 *      implemented there (V0.9.16).
 *
 *      If no error occurs, NO_ERROR is returned
 *      and a pointer to a new EXECUTABLE structure
 *      is stored in *ppExec. Consider this pointer a
 *      handle and pass it to exehClose to clean up.
 *
 *      If NO_ERROR is returned, all the fields through
 *      ulOS are set in EXECUTABLE. The psz* fields
 *      which follow afterwards require an additional
 *      call to exehQueryBldLevel.
 *
 *      NOTE: If NO_ERROR is returned, the executable
 *      file is kept open by this function. It will
 *      only be closed when you call exehClose.
 *
 *      If errors occur, this function returns the
 *      following error codes:
 *
 *      -- ERROR_NOT_ENOUGH_MEMORY: malloc() failed.
 *
 *      -- ERROR_INVALID_EXE_SIGNATURE (191): header is
 *              neither plain DOS, nor NE, nor LX, nor PE.
 *              The given file probably isn't even an
 *              executable.
 *
 *      -- ERROR_BAD_EXE_FORMAT (193): header was
 *              recognized, but the header data was
 *              not understood. Also this might be
 *              returned for .COM files which are
 *              not recognized.
 *
 *      -- ERROR_INVALID_PARAMETER: ppExec is NULL.
 *
 *      plus those of doshOpen and doshReadAt.
 *
 *      The following executable types are supported
 *      (see EXECUTABLE for details):
 *
 *      --  Plain DOS 3.x executable without new header.
 *
 *      --  New Executable (NE), used by Win16 and
 *          16-bit OS/2 and still many of today's drivers.
 *
 *      --  Linear Executable (LX), OS/2 2.x and above.
 *
 *      --  Portable Executable (PE), used by Win32.
 *
 *      --  For files with the .COM, .BAT, or .CMD
 *          extensions, this will _not_ open the
 *          executable file, but set flags in EXECUTABLE
 *          only. Most importantly, EXECUTABLE.pDosExeHeader
 *          will be NULL.
 *
 *      V0.9.12 adds support for NOSTUB executables,
 *      which are new-style executables (NE or LX)
 *      without a leading DOS header. JFS.IFS uses that
 *      format, for example. The executable then starts
 *      directly with the NE or LX header. I am not sure
 *      whether PE supports such beasts  as well... if
 *      so, it should be supported too.
 *
 *      Note that not all of the other exeh* functions
 *      support all of the executable types. See the
 *      respective function descriptions for remarks.
 *
 *      @@todo:
 *
 *          win95 \WINDOWS\extract.exe is NE with a non-standard format
 *          win16 \WINDOWS\EXPAND.EXE
 *          win16 \WINDOWS\MSD.EXE"
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.1 (2000-02-13) [umoeller]: fixed 32-bits flag
 *@@changed V0.9.7 (2000-12-20) [lafaix]: fixed ulNewHeaderOfs
 *@@changed V0.9.10 (2001-04-08) [lafaix]: added PE support
 *@@changed V0.9.10 (2001-04-08) [umoeller]: now setting ppExec only if NO_ERROR is returned
 *@@changed V0.9.12 (2001-05-03) [umoeller]: added support for NOSTUB newstyle executables
 *@@changed V0.9.16 (2001-12-08) [umoeller]: now using OPEN_SHARE_DENYWRITE
 *@@changed V0.9.16 (2001-12-08) [umoeller]: fLibrary was never set, works for LX, NE, and PE now
 *@@changed V0.9.16 (2001-12-08) [umoeller]: speed optimizations, changed some return codes
 *@@changed V0.9.16 (2002-01-04) [umoeller]: added fixes for COM, BAT, CMD extensions
 *@@changed V1.0.0 (2002-08-18) [umoeller]: this was completely broken for files without extensions (os2krnl)
 */

APIRET exehOpen(const char* pcszExecutable,
                PEXECUTABLE* ppExec)
{
    APIRET      arc = NO_ERROR;

    PEXECUTABLE pExec = NULL;

    PXFILE      pFile = NULL;
    ULONG       cbFile = 0;
    PCSZ        pExt;
    BOOL        fOpenFile = FALSE;
    BOOL        fLoadNewHeader = FALSE;
    ULONG       ulNewHeaderOfs = 0;       // V0.9.12 (2001-05-03) [umoeller]

    if (!ppExec)
        return ERROR_INVALID_PARAMETER;

    if (!(pExec = (PEXECUTABLE)malloc(sizeof(EXECUTABLE))))
        return ERROR_NOT_ENOUGH_MEMORY;

    memset(pExec, 0, sizeof(EXECUTABLE));

    // check some of the default extensions
    // V0.9.16 (2002-01-04) [umoeller]
    if (!(pExt = doshGetExtension(pcszExecutable)))
    {
        // has no extension: then open file!
        // fixed V1.0.0 (2002-08-18) [umoeller]
        fOpenFile = TRUE;
    }
    else
    {
        // has extension:
        if (!stricmp(pExt, "COM"))
        {
            // I am not willing to find out more about the
            // .COM executable format, so for this one case,
            // let OS/2 determine what we have here
            // (otherwise we do _not_ use DosQueryAppType
            // because it's quite an expensive call)
            ULONG ulDosAppType = 0;
            if (!(arc = DosQueryAppType((PSZ)pcszExecutable, &ulDosAppType)))
            {
                if (ulDosAppType & FAPPTYP_DOS)           // 0x20
                    pExec->ulOS = EXEOS_DOS3;
                else
                {
                    ULONG fl = ulDosAppType & FAPPTYP_WINDOWAPI; // 0x03
                    if (    (fl == FAPPTYP_WINDOWCOMPAT) // 0x02)
                         || (fl == FAPPTYP_NOTWINDOWCOMPAT) // 0x01)
                       )
                        pExec->ulOS = EXEOS_OS2;
                    else
                        arc = ERROR_BAD_EXE_FORMAT;
                }

                pExec->ulExeFormat = EXEFORMAT_COM;
            }
        }
        else if (!stricmp(pExt, "BAT"))
        {
            pExec->ulOS = EXEOS_DOS3;
            pExec->ulExeFormat = EXEFORMAT_TEXT_BATCH;
        }
        else if (!stricmp(pExt, "CMD"))
        {
            pExec->ulOS = EXEOS_OS2;
            pExec->ulExeFormat = EXEFORMAT_TEXT_CMD;
        }
        else
            fOpenFile = TRUE;
    }

    if (    (fOpenFile)     // none of the above
         && (!(arc = doshOpen((PSZ)pcszExecutable,
                              XOPEN_READ_EXISTING,
                              &cbFile,
                              &pFile)))
       )
    {
        // file opened successfully:
        pExec->pFile = pFile;
        pExec->cbDosExeHeader = sizeof(DOSEXEHEADER);

        // read old DOS EXE header
        if ((arc = doshReadAt(pFile,
                              0,
                              &pExec->cbDosExeHeader,      // in/out
                              (PBYTE)&pExec->DosExeHeader,
                              DRFL_FAILIFLESS)))
            pExec->cbDosExeHeader = 0;
        else
        {
            // now check if we really have a DOS header
            if (pExec->DosExeHeader.usDosExeID != 0x5a4d)
            {
                // arc = ERROR_INVALID_EXE_SIGNATURE;

                // V0.9.12 (2001-05-03) [umoeller]
                // try loading new header directly; there are
                // drivers which were built with NOSTUB, and
                // the exe image starts out with the NE or LX
                // image directly (try JFS.IFS)
                fLoadNewHeader = TRUE;
                        // ulNewHeaderOfs is 0 now

                // remove the DOS header info, since we have none
                // V0.9.12 (2001-05-03) [umoeller]
                // FREE(pExec->pDosExeHeader);
                pExec->cbDosExeHeader = 0;
            }
            else
            {
                // V1.0.3 (2004-10-24) [pr]: Some non-DOS EXEs have a relocation table
                // offset which is 0 - these were previously identified as DOS EXEs.
                // we have a DOS header:
                if (   (   (pExec->DosExeHeader.usRelocTableOfs == 0)
                        || (pExec->DosExeHeader.usRelocTableOfs >= sizeof(DOSEXEHEADER))
                       )
                    && (pExec->DosExeHeader.ulNewHeaderOfs != 0)
                   )
                {
                    // we have a new header offset:
                    fLoadNewHeader = TRUE;
                    ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;
                }
                else
                {
                    // else DOS:
                    pExec->ulOS = EXEOS_DOS3;
                    pExec->ulExeFormat = EXEFORMAT_OLDDOS;
                }
            }
        }
    }

    if (fLoadNewHeader)
    {
        // either LX or PE or NE:
        // read in new header...
        // ulNewHeaderOfs is now either 0 (if no DOS header
        // was found) or pDosExeHeader->ulNewHeaderOfs
        // V0.9.12 (2001-05-03) [umoeller]

        // read in the first two bytes to find out
        // what extended header type we have; note,
        // PE uses four bytes here
        CHAR    achNewHeaderType[4] = "???";
        ULONG   cbRead = 4;

        if (!(arc = doshReadAt(pFile,
                               ulNewHeaderOfs,
                               &cbRead,
                               achNewHeaderType,
                               DRFL_FAILIFLESS)))
        {
            PBYTE   pbCheckOS = NULL;

            if (!memcmp(achNewHeaderType, "NE", 2))
            {
                // New Executable:
                pExec->ulExeFormat = EXEFORMAT_NE;
                cbRead = sizeof(NEHEADER);

                // go read in the complete header then
                // (doshReadAt has this in the cache)
                if (!(pExec->pNEHeader = (PNEHEADER)malloc(cbRead)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else if (!(arc = doshReadAt(pFile,
                                            ulNewHeaderOfs,
                                            &cbRead,
                                            (PBYTE)pExec->pNEHeader,
                                            0)))
                {
                    if (cbRead < sizeof(NEHEADER))
                        arc = ERROR_BAD_EXE_FORMAT;
                    else
                    {
                        pExec->cbNEHeader = cbRead;
                        pbCheckOS = &pExec->pNEHeader->bTargetOS;
                        // set library flag V0.9.16 (2001-12-08) [umoeller]
                        if (pExec->pNEHeader->usFlags & 0x8000)
                            // library:
                            pExec->fLibrary = TRUE;
                    }
                }
            }
            else if (    (!memcmp(achNewHeaderType, "LX", 2))
                      || (!memcmp(achNewHeaderType, "LE", 2))
                                // this is used by SMARTDRV.EXE
                    )
            {
                // OS/2 Linear Executable:
                pExec->ulExeFormat = EXEFORMAT_LX;
                cbRead = sizeof(LXHEADER);

                // go read in the complete header then
                // (doshReadAt has this in the cache)
                if (!(pExec->pLXHeader = (PLXHEADER)malloc(cbRead)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else if (!(arc = doshReadAt(pFile,
                                            ulNewHeaderOfs,
                                            &cbRead,
                                            (PBYTE)pExec->pLXHeader,
                                            0)))
                {
                    if (cbRead < sizeof(LXHEADER))
                        arc = ERROR_BAD_EXE_FORMAT;
                    else
                    {
                        pExec->cbLXHeader = cbRead;
                        pbCheckOS = (PBYTE)(&pExec->pLXHeader->usTargetOS);
                        // set library flag V0.9.16 (2001-12-08) [umoeller]
                        if (pExec->pLXHeader->ulFlags & 0x8000)
                            // library:
                            pExec->fLibrary = TRUE;
                    }
                }
            }
            else if (!memcmp(achNewHeaderType, "PE\0\0", 4))
            {
                pExec->ulExeFormat = EXEFORMAT_PE;

                // PE has a standard header of 24 bytes
                // plus an extended header, so check
                // what we've got
                if (!(pExec->pPEHeader = (PPEHEADER)malloc(sizeof(PEHEADER))))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    ULONG ulOfs = ulNewHeaderOfs + 4;
                    PPEHEADER pPEHeader = pExec->pPEHeader;

                    // null the entire header
                    memset(pExec->pPEHeader,
                           0,
                           sizeof(PEHEADER));

                    // copy sig
                    pPEHeader->ulSignature = *((PULONG)&achNewHeaderType);

                    // read standard header
                    cbRead = sizeof(IMAGE_FILE_HEADER);
                    if (!(arc = doshReadAt(pFile,
                                           ulOfs,
                                           &cbRead,
                                           (PBYTE)&pPEHeader->FileHeader,
                                           0)))
                    {
                        if (cbRead < sizeof(IMAGE_FILE_HEADER))
                            // only if we don't even have the
                            // standard header, return an error
                            arc = ERROR_BAD_EXE_FORMAT;
                        else
                        {
                            pExec->f32Bits = TRUE;
                            pExec->cbPEHeader = 4 + sizeof(PEHEADER);       // for now

                            if (pPEHeader->FileHeader.fsCharacteristics & IMAGE_FILE_DLL)
                                pExec->fLibrary = TRUE;

                            // try extended header
                            ulOfs += sizeof(IMAGE_FILE_HEADER);
                            if (    (cbRead = pPEHeader->FileHeader.usSizeOfOptionalHeader)
                                 && (cbRead <= sizeof(IMAGE_OPTIONAL_HEADER))
                               )
                            {
                                if (!(arc = doshReadAt(pFile,
                                                       ulOfs,
                                                       &cbRead,
                                                       (PBYTE)&pPEHeader->OptionalHeader,
                                                       0)))
                                {
                                    if (cbRead != sizeof(IMAGE_OPTIONAL_HEADER))
                                        arc = ERROR_BAD_EXE_FORMAT;
                                    else switch (pPEHeader->OptionalHeader.usSubsystem)
                                    {
                                        // case IMAGE_SUBSYSTEM_UNKNOWN:   // 0
                                        // case IMAGE_SUBSYSTEM_NATIVE:    // 1
                                        // case IMAGE_SUBSYSTEM_OS2_CUI:   // 5
                                        // case IMAGE_SUBSYSTEM_POSIX_CUI: // 7
                                                // for these we shouldn't set win32

                                        case IMAGE_SUBSYSTEM_WINDOWS_GUI: // 2   // Windows GUI subsystem
                                            pExec->ulOS = EXEOS_WIN32_GUI;
                                        break;

                                        case IMAGE_SUBSYSTEM_WINDOWS_CUI: // 3   // Windows character subsystem
                                            pExec->ulOS = EXEOS_WIN32_CLI;
                                        break;
                                    }

                                    pExec->cbPEHeader = sizeof(PEHEADER);
                                }
                            }
                        } // end else if (cbRead < sizeof(IMAGE_FILE_HEADER))
                    } // end if (!(arc = doshReadAt(pFile,
                } // end else if (!(pExec->pPEHeader = (PPEHEADER)malloc(sizeof(PEHEADER))))
            } // end else if (!memcmp(achNewHeaderType, "PE\0\0", 4))
            else
                // strange type:
                arc = ERROR_INVALID_EXE_SIGNATURE;

            if ((!arc) && (pbCheckOS))
            {
                // BYTE to check for operating system
                // (NE and LX):
                switch (*pbCheckOS)
                {
                    case NEOS_OS2:
                        pExec->ulOS = EXEOS_OS2;
                        if (pExec->ulExeFormat == EXEFORMAT_LX)
                            pExec->f32Bits = TRUE;
                    break;

                    case NEOS_WIN16:
                        pExec->ulOS = EXEOS_WIN16;
                    break;

                    case NEOS_DOS4:
                        pExec->ulOS = EXEOS_DOS4;
                    break;

                    case NEOS_WIN386:
                        pExec->ulOS = EXEOS_WIN386;
                        pExec->f32Bits = TRUE;
                    break;
                }
            }
        } // end if (!(arc = doshReadAt(hFile,
    } // end if (fLoadNewHeader)

    if (arc)
        // error: clean up
        exehClose(&pExec);
    else
        *ppExec = pExec;

    return arc;
}

/*
 *@@ ParseBldLevel:
 *      called from exehQueryBldLevel to parse the BLDLEVEL string.
 *
 *      On entry, caller has copied the string into pExec->pszDescription.
 *      The string is null-terminated.
 *
 *      The BLDLEVEL string comes in at least two basic flavors.
 *
 *      --  The standard format is:
 *
 +              @#VENDOR:VERSION#@DESCRIPTION
 *
 *          DESCRIPTION can have leading spaces, but
 *          need to have them.
 *
 *      --  However, there is an extended version in that the DESCRIPTION field
 *          is split up even more.
 *
 *          I have seen two subformats for this.
 *
 *          --  DANIS506.ADD and some IBM programs have a marker that seems
 *              to be that the description starts out with "##1##".
 *
 +              ##1## DATETIME BUILDMACHINE:ASD:LANG:CTRY:REVISION:UNKNOWN:FIXPAK@@DESCRIPTION
 *
 *              The problem is that the DATETIME field comes
 *              in several flavors. IBM uses things like
 *
 +                  "Thu Nov 30 15:30:37 2000 BWBLD228"
 *
 *              while DANIS506.ADD has
 *
 +                  "15.12.2000 18:22:57      Nachtigall"
 *
 *              Looks like the date/time string is standardized to have 24 characters then.
 *
 *          --  IBM TCP/IP executables (try INETD.EXE) have something yet different on.
 *              We now try to parse that format as well, even though bldlevel.exe can't
 *              handle it. (Isn't that utility from IBM too?)
 *
 *              Here's what I get for inetd.exe:
 *
 +              ##built 09:16:27 Mon Sep 17 2001 -- On AURORA43;0.1@@ TCP/IP for OS/2: INETD
 *
 *@@added V0.9.12 (2001-05-18) [umoeller]
 *@@changed V0.9.12 (2001-05-19) [umoeller]: added extended BLDLEVEL support
 *@@changed V1.0.0 (2002-08-18) [umoeller]: added support for IBM TCP/IP format
 *@@changed V1.0.0 (2002-08-18) [umoeller]: fixed DANIS506 format when an extended field had only one character
 */

STATIC VOID ParseBldLevel(PEXECUTABLE pExec)
{
    PCSZ    pStartOfVendor,
            pStartOfInfo,
            pEndOfVendor;

    // @#VENDOR:VERSION#@ DESCRIPTION
    if (    (pStartOfVendor = strstr(pExec->pszDescription, "@#"))
         && (pStartOfInfo = strstr(pStartOfVendor + 2, "#@"))
         && (pEndOfVendor = strchr(pStartOfVendor + 2, ':'))
       )
    {
        pExec->pszVendor = strhSubstr(pStartOfVendor + 2,
                                      pEndOfVendor);
        pExec->pszVersion = strhSubstr(pEndOfVendor + 1,
                                       pStartOfInfo);
        // skip "@#" in DESCRIPTION string
        pStartOfInfo += 2;

        // now check if we have extended DESCRIPTION V0.9.12 (2001-05-19) [umoeller]
        if (strlen(pStartOfInfo) > 6)
        {
            if (!memcmp(pStartOfInfo, "##1##", 5))
            {
                // DANIS506.ADD format:
                // "##1## 2.7.2002 19:32:34        Nachtigall::::6::@@..."

                // parse that beast
                PCSZ p = pStartOfInfo + 5;

                // get build date/time
                if (strlen(p) > 24)
                {
                    // skip leading and trailing spaces
                    // V1.0.0 (2002-08-18) [umoeller]
                    PCSZ pStartOfDT = p,
                         pEndOfDT = p + 24;
                                // date/time seems to be fixed 24 chars in length

                    while (*pStartOfDT == ' ')
                        ++pStartOfDT;

                    while (    (*pEndOfDT == ' ')
                            && (pEndOfDT > pStartOfDT)
                          )
                        --pEndOfDT;

                    pExec->pszBuildDateTime = strhSubstr(pStartOfDT, pEndOfDT + 1);

                    /* memcpy(pExec->pszBuildDateTime,
                           p,
                           24);
                    pExec->pszBuildDateTime[24] = '\0';
                    */

                    // date/time seems to be fixed 24 chars in length
                    p += 24;

                    // now we're at the colon-separated
                    // strings, first of which is the build machine;
                    // skip leading spaces
                    while (*p == ' ')
                        p++;

                    if (*p)
                    {
                        ULONG ul;
                        char **papsz[7];
                        papsz[0]=&pExec->pszBuildMachine;
                        papsz[1]=&pExec->pszASD;
                        papsz[2]=&pExec->pszLanguage;
                        papsz[3]=&pExec->pszCountry;
                        papsz[4]=&pExec->pszRevision;
                        papsz[5]=&pExec->pszUnknown;
                        papsz[6]=&pExec->pszFixpak;


                        for (ul = 0;
                             ul < sizeof(papsz) / sizeof(papsz[0]);
                             ul++)
                        {
                            BOOL    fStop = FALSE;
                            PCSZ    pNextColon = strchr(p, ':'),
                                    pDoubleAt = strstr(p, "@@");
                            if (!pNextColon)
                            {
                                // last item:
                                if (pDoubleAt)
                                    pNextColon = pDoubleAt;
                                else
                                    pNextColon = p + strlen(p);

                                fStop = TRUE;
                            }

                            if (    (fStop)
                                 || (    (pNextColon)
                                      && (    (!pDoubleAt)
                                           || (pNextColon < pDoubleAt)
                                         )
                                    )
                               )
                            {
                                // if (pNextColon > p + 1)
                                        // fixed V1.0.0 (2002-08-18) [umoeller]
                                        // this failed on fields like "revision"
                                        // which only had one character
                                if (pNextColon > p)
                                    *(papsz[ul]) = strhSubstr(p, pNextColon);
                            }
                            else
                                break;

                            if (fStop)
                                break;

                            p = pNextColon + 1;
                        }
                    }
                }

                if (pStartOfInfo = strstr(p,
                                          "@@"))
                    pStartOfInfo += 2;
            } // end if (!memcmp(pStartOfInfo, "##1##", 5))
            else if (!memcmp(pStartOfInfo, "##built", 7))
            {
                // IBM TCP/IP format:
                // V1.0.0 (2002-08-18) [umoeller]

                // ##built 09:16:27 Mon Sep 17 2001 -- On AURORA43;0.1@@ TCP/IP for OS/2: INETD

                PCSZ    p = pStartOfInfo + 7,
                        p2,
                        p3;

                if (p3 = strchr(p, ';'))
                {
                    while (*p == ' ')
                        ++p;

                    // ##built 09:16:27 Mon Sep 17 2001 -- On AURORA43;0.1@@ TCP/IP for OS/2: INETD
                    //         ^ p                                    ^ p3
                    //                                 ^ p2

                    if (    (p2 = strstr(p, " -- On "))
                         && (p2 < p3)
                       )
                    {
                        pExec->pszBuildMachine = strhSubstr(p2 + 7, p3);
                        pExec->pszBuildDateTime = strhSubstr(p, p2);
                    }
                    else
                        pExec->pszBuildDateTime = strhSubstr(p, p3);

                    p = p3 + 1;
                }

                if (pStartOfInfo = strstr(p,
                                          "@@"))
                {
                    if (pStartOfInfo > p3)
                    {
                        // p3 points to this "0.1" string; I assume this is
                        // a "revision.fixpak" format since inetver reports
                        // four digits with this revision
                        PCSZ p4;
                        if (    (p4 = strchr(p3, '.'))
                             && (p4 < pStartOfInfo)
                           )
                        {
                            pExec->pszRevision = strhSubstr(p3 + 1, p4);
                            pExec->pszFixpak = strhSubstr(p4 + 1, pStartOfInfo);
                        }
                        else
                            pExec->pszRevision = strhSubstr(p3 + 1, pStartOfInfo);
                    }

                    pStartOfInfo += 2;
                }
            }
        }

        // -- if we had no extended DESCRIPTION,
        //    pStartOfInfo points to regular description now
        // -- if we parse the extended DESCRIPTION above,
        //    pStartOfInfo points to after @@ now
        // -- if we had an error, pStartOfInfo is NULL
        if (pStartOfInfo)
        {
            // add the regular DESCRIPTION then
            // skip leading spaces in info string
            while (*pStartOfInfo == ' ')
                ++pStartOfInfo;

            if (*pStartOfInfo)  // V0.9.9 (2001-04-04) [umoeller]
                // and copy until end of string
                pExec->pszInfo = strdup(pStartOfInfo);
        }
    }
}

/*
 *@@ exehQueryBldLevel:
 *      this retrieves buildlevel information for an
 *      LX or NE executable previously opened with
 *      exehOpen.
 *
 *      BuildLevel information must be contained in the
 *      DESCRIPTION field of an executable's module
 *      definition (.DEF) file. In order to be readable
 *      by BLDLEVEL.EXE (which ships with OS/2), this
 *      string must have the following format:
 *
 +          Description '@#AUTHOR:VERSION#@ DESCRIPTION'
 *
 *      Example:
 *
 +          Description '@#Ulrich M”ller:0.9.0#@ XWorkplace Sound Support Module'
 *
 *      The "Description" entry always ends up as the
 *      very first entry in the non-resident name table
 *      in LX and NE executables. So this is what we retrieve
 *      here.
 *
 *      If the first entry in that table exists, NO_ERROR is
 *      returned and at least the pszDescription field in
 *      EXECUTABLE is set to that information.
 *
 *      If that string is in IBM BLDLEVEL format, the string
 *      is automatically parsed, and the pszVendor, pszVersion,
 *      and pszInfo fields are also set. In the above examples,
 *      this would return the following information:
 *
 +          pszVendor = "Ulrich M”ller"
 +          pszVersion = "0.9.0"
 +          pszInfo = "XWorkplace Sound Support Module"
 *
 *      See ParseBldLevel for extended formats.
 *
 *      If that string is not in BLDLEVEL format, only
 *      pszDescription will be set. The other fields remain
 *      NULL. Still, NO_ERROR is returned.
 *
 *      This returns the following errors:
 *
 *      -- ERROR_INVALID_PARAMETER: pExec is NULL.
 *
 *      -- ERROR_INVALID_EXE_SIGNATURE (191): pExec is not in
 *                  LX or NE format.
 *
 *      -- ERROR_INVALID_DATA (13): non-resident name table not found,
 *                  or table is empty.
 *
 *      -- ERROR_NOT_ENOUGH_MEMORY: malloc() failed.
 *
 *      plus the error codes of doshReadAt.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.0 (99-10-22) [umoeller]: NE format now supported
 *@@changed V0.9.1 (99-12-06): fixed memory leak
 *@@changed V0.9.9 (2001-04-04) [umoeller]: added more error checking
 *@@changed V0.9.12 (2001-05-18) [umoeller]: extracted ParseBldLevel
 *@@changed V0.9.16 (2002-01-05) [umoeller]: optimizations
 */

APIRET exehQueryBldLevel(PEXECUTABLE pExec)
{
    APIRET      arc = NO_ERROR;
    PXFILE      pFile;
    ULONG       ulNRNTOfs = 0;

    if (!pExec)
        return ERROR_INVALID_PARAMETER;

    pFile = pExec->pFile;
    if (pExec->ulExeFormat == EXEFORMAT_LX)
    {
        // OK, LX format:
        // check if we have a non-resident name table
        if (pExec->pLXHeader == NULL)
            arc = ERROR_INVALID_DATA;
        else if (pExec->pLXHeader->ulNonResdNameTblOfs == 0)
            arc = ERROR_INVALID_DATA;
        else
            ulNRNTOfs = pExec->pLXHeader->ulNonResdNameTblOfs;
    }
    else if (pExec->ulExeFormat == EXEFORMAT_NE)
    {
        // OK, NE format:
        // check if we have a non-resident name table
        if (pExec->pNEHeader == NULL)
            arc = ERROR_INVALID_DATA;
        else if (pExec->pNEHeader->ulNonResdTblOfs == 0)
            arc = ERROR_INVALID_DATA;
        else
            ulNRNTOfs = pExec->pNEHeader->ulNonResdTblOfs;
    }
    else
        // neither LX nor NE: stop
        arc = ERROR_INVALID_EXE_SIGNATURE;

    if (    (!arc)
         && (ulNRNTOfs)
       )
    {
        ULONG       cb = 2000;

        PSZ         pszNameTable;

        if (!(pszNameTable = (PSZ)malloc(2001)))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            // V0.9.16 (2002-01-05) [umoeller]: rewrote the following

            // read from offset of non-resident name table
            if (!(arc = doshReadAt(pFile,           // file is still open
                                   ulNRNTOfs,       // ofs determined above
                                   &cb,             // 2000
                                   pszNameTable,
                                   0)))
            {
                // the string is in Pascal format, so the
                // first byte has the length
                BYTE bLen;
                if (!(bLen = *pszNameTable))
                    // length byte is null:
                    arc = ERROR_INVALID_DATA;
                else
                {
                    // now copy the string
                    if (!(pExec->pszDescription = (PSZ)malloc(bLen + 1)))
                        arc = ERROR_NOT_ENOUGH_MEMORY;
                    else
                    {
                        memcpy(pExec->pszDescription,
                               pszNameTable + 1,    // skip length byte
                               bLen);               // length byte
                        // terminate string
                        pExec->pszDescription[bLen] = 0;

                        ParseBldLevel(pExec);
                    }
                }
            }

            free(pszNameTable);
        }
    }

    return arc;
}

/*
 *@@ exehQueryProgType:
 *      attempts to sets *pulProgType to a PROGTYPE constant.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_INVALID_PARAMETER;
 *
 *@@added V1.0.1 (2003-01-17) [umoeller]
 *@@changed V1.0.1 (2003-01-17) [umoeller]: now correctly returning PROG_PDD/VDD for NE and LX @@fixes 343
 */

APIRET exehQueryProgType(const EXECUTABLE *pExec,
                         PROGCATEGORY *pulProgType)
{
    APIRET      arc = NO_ERROR;

    if (!pExec)
        return ERROR_INVALID_PARAMETER;

    // now we have the PEXECUTABLE:
    // check what we found
    switch (pExec->ulOS)
    {
        case EXEOS_DOS3:
        case EXEOS_DOS4:
            *pulProgType = PROG_WINDOWEDVDM;
        break;

        case EXEOS_OS2:
            switch (pExec->ulExeFormat)
            {
                case EXEFORMAT_LX:
                    switch (pExec->pLXHeader->ulFlags & E32MODMASK)
                    {
                        case E32MODPDEV:
                            *pulProgType = PROG_PDD;
                        break;

                        case E32MODVDEV:
                            *pulProgType = PROG_VDD;
                        break;

                        case E32MODDLL:
                        case E32MODPROTDLL:
                            *pulProgType = PROG_DLL;
                        break;

                        default:
                            // all bits clear: --> real executable
                            switch (pExec->pLXHeader->ulFlags & E32APPMASK)
                            {
                                case E32PMAPI:
                                    // _Pmpf(("  LX OS2 PM"));
                                    *pulProgType = PROG_PM;
                                break;

                                case E32PMW:
                                    // _Pmpf(("  LX OS2 VIO"));
                                    *pulProgType = PROG_WINDOWABLEVIO;
                                break;

                                case E32NOPMW:
                                    // _Pmpf(("  LX OS2 FULLSCREEN"));
                                    *pulProgType = PROG_FULLSCREEN;
                                break;

                                default:
                                    // _Pmpf(("  LX OS2 FULLSCREEN"));
                                    *pulProgType = PROG_FULLSCREEN;
                                break;
                            }
                        break;      // executable
                    }
                break;

                case EXEFORMAT_NE:
                    if (pExec->fLibrary)
                    {
                        // there is no flag in the NE header for whether
                        // this is a device driver, so rely on extension
                        // V1.0.1 (2003-01-17) [umoeller]
                        PSZ p;
                        if (    (p = doshGetExtension(pExec->pFile->pszFilename))
                             && (    (!stricmp(p, "ADD"))
                                  || (!stricmp(p, "DMD"))
                                  || (!stricmp(p, "FLT"))
                                  || (!stricmp(p, "IFS"))
                                  || (!stricmp(p, "SNP"))
                                  || (!stricmp(p, "SYS"))
                                )
                           )
                            *pulProgType = PROG_PDD;
                                    // there can be no 16-bit VDDs, so this must be a PDD
                        else
                            *pulProgType = PROG_DLL;
                    }
                    else switch (pExec->pNEHeader->usFlags & NEAPPTYP)
                    {
                        case NEWINCOMPAT:
                            // _Pmpf(("  NE OS2 VIO"));
                            *pulProgType = PROG_WINDOWABLEVIO;
                        break;

                        case NEWINAPI:
                            // _Pmpf(("  NE OS2 PM"));
                            *pulProgType = PROG_PM;
                        break;

                        case NENOTWINCOMPAT:
                        default:
                            // _Pmpf(("  NE OS2 FULLSCREEN"));
                            *pulProgType = PROG_FULLSCREEN;
                        break;
                    }
                break;

                case EXEFORMAT_COM:
                    *pulProgType = PROG_WINDOWABLEVIO;
                break;

                default:
                    arc = ERROR_INVALID_EXE_SIGNATURE;
            }
        break;

        case EXEOS_WIN16:
        case EXEOS_WIN386:
            // _Pmpf(("  WIN16"));
            *pulProgType = PROG_31_ENHSEAMLESSCOMMON;
        break;

        case EXEOS_WIN32_GUI:
        case EXEOS_WIN32_CLI:
            // _Pmpf(("  WIN32"));
            *pulProgType = PROG_WIN32;
        break;

        default:
            arc = ERROR_INVALID_EXE_SIGNATURE;
    }

    return arc;
}

/*
 *@@ PROGTYPESTRING:
 *
 *@@added V0.9.16 (2002-01-13) [umoeller]
 */

typedef struct _PROGTYPESTRING
{
    PROGCATEGORY    progc;
    PCSZ            pcsz;
} PROGTYPESTRING, *PPROGTYPESTRING;

PROGTYPESTRING G_aProgTypes[] =
    {
        PROG_DEFAULT, "PROG_DEFAULT",
        PROG_FULLSCREEN, "PROG_FULLSCREEN",
        PROG_WINDOWABLEVIO, "PROG_WINDOWABLEVIO",
        PROG_PM, "PROG_PM",
        PROG_GROUP, "PROG_GROUP",
        PROG_VDM, "PROG_VDM",
            // same as PROG_REAL, "PROG_REAL",
        PROG_WINDOWEDVDM, "PROG_WINDOWEDVDM",
        PROG_DLL, "PROG_DLL",
        PROG_PDD, "PROG_PDD",
        PROG_VDD, "PROG_VDD",
        PROG_WINDOW_REAL, "PROG_WINDOW_REAL",
        PROG_30_STD, "PROG_30_STD",
            // same as PROG_WINDOW_PROT, "PROG_WINDOW_PROT",
        PROG_WINDOW_AUTO, "PROG_WINDOW_AUTO",
        PROG_30_STDSEAMLESSVDM, "PROG_30_STDSEAMLESSVDM",
            // same as PROG_SEAMLESSVDM, "PROG_SEAMLESSVDM",
        PROG_30_STDSEAMLESSCOMMON, "PROG_30_STDSEAMLESSCOMMON",
            // same as PROG_SEAMLESSCOMMON, "PROG_SEAMLESSCOMMON",
        PROG_31_STDSEAMLESSVDM, "PROG_31_STDSEAMLESSVDM",
        PROG_31_STDSEAMLESSCOMMON, "PROG_31_STDSEAMLESSCOMMON",
        PROG_31_ENHSEAMLESSVDM, "PROG_31_ENHSEAMLESSVDM",
        PROG_31_ENHSEAMLESSCOMMON, "PROG_31_ENHSEAMLESSCOMMON",
        PROG_31_ENH, "PROG_31_ENH",
        PROG_31_STD, "PROG_31_STD",

// Warp 4 toolkit defines, whatever these were designed for...
#ifndef PROG_DOS_GAME
    #define PROG_DOS_GAME            (PROGCATEGORY)21
#endif
#ifndef PROG_WIN_GAME
    #define PROG_WIN_GAME            (PROGCATEGORY)22
#endif
#ifndef PROG_DOS_MODE
    #define PROG_DOS_MODE            (PROGCATEGORY)23
#endif

        PROG_DOS_GAME, "PROG_DOS_GAME",
        PROG_WIN_GAME, "PROG_WIN_GAME",
        PROG_DOS_MODE, "PROG_DOS_MODE",

        // added this V0.9.16 (2001-12-08) [umoeller]
        PROG_WIN32, "PROG_WIN32"
    };

/*
 *@@ exehDescribeProgType:
 *      returns a "PROG_*" string for the given
 *      program type. Useful for WPProgram setup
 *      strings and such.
 *
 *@@added V0.9.16 (2001-10-06)
 *@@changed V1.0.1 (2003-01-17) [umoeller]: moved this here from apps.c
 */

PCSZ exehDescribeProgType(PROGCATEGORY progc)        // in: from PROGDETAILS.progc
{
    ULONG ul;
    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aProgTypes);
         ul++)
    {
        if (G_aProgTypes[ul].progc == progc)
            return G_aProgTypes[ul].pcsz;
    }

    return NULL;
}

/*
 *@@ exehQueryImportedModules:
 *      returns an array of FSYSMODULE structure describing all
 *      imported modules.
 *
 *      *pcModules receives the # of items in the array (not the
 *      array size!).  Use doshFreeImportedModules to clean up.
 *
 *      This returns a standard OS/2 error code, which might be
 *      any of the codes returned by DosSetFilePtr and DosRead.
 *      In addition, this may return:
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: exe is in a format other
 *          than LX or NE, which is not understood by this function.
 *
 *      Even if NO_ERROR is returned, the array pointer might still
 *      be NULL if the module contains no such data.
 *
 *@@added V0.9.9 (2001-03-11) [lafaix]
 *@@changed V0.9.9 (2001-04-03) [umoeller]: added tons of error checking, changed prototype to return APIRET
 *@@changed V0.9.9 (2001-04-05) [lafaix]: rewritten error checking code
 *@@changed V0.9.10 (2001-04-10) [lafaix]: added Win16 and Win386 support
 *@@changed V0.9.10 (2001-04-13) [lafaix]: removed 127 characters limit
 *@@changed V0.9.12 (2001-05-03) [umoeller]: adjusted for new NOSTUB support
 */

APIRET exehQueryImportedModules(PEXECUTABLE pExec,
                                PFSYSMODULE *ppaModules,    // out: modules array
                                PULONG pcModules)           // out: array item count
{
    if (    (pExec)
         && (    (pExec->ulOS == EXEOS_OS2)
              || (pExec->ulOS == EXEOS_WIN16)
              || (pExec->ulOS == EXEOS_WIN386)
            )
       )
    {
        ENSURE_BEGIN;
        ULONG       cModules = 0;
        PFSYSMODULE paModules = NULL;
        int i;
        HFILE hfExe = pExec->pFile->hf;

        ULONG ulNewHeaderOfs = 0;       // V0.9.12 (2001-05-03) [umoeller]

        if (pExec->cbDosExeHeader)
            // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
            ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

        if (pExec->ulExeFormat == EXEFORMAT_LX)
        {
            // 32-bit OS/2 executable:
            cModules = pExec->pLXHeader->ulImportModTblCnt;

            if (cModules)
            {
                ULONG   cb = sizeof(FSYSMODULE) * cModules; // V0.9.9 (2001-04-03) [umoeller]
                ULONG   ulDummy;

                paModules = (PFSYSMODULE)malloc(cb);
                if (!paModules)
                    ENSURE_FAIL(ERROR_NOT_ENOUGH_MEMORY); // V0.9.9 (2001-04-03) [umoeller]

                memset(paModules, 0, cb);   // V0.9.9 (2001-04-03) [umoeller]

                ENSURE_SAFE(DosSetFilePtr(hfExe,
                                          pExec->pLXHeader->ulImportModTblOfs
                                            + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                                          FILE_BEGIN,
                                          &ulDummy));

                for (i = 0; i < cModules; i++)
                {
                    BYTE bLen = 0;

                    // reading the length of the module name
                    ENSURE_SAFE(DosRead(hfExe, &bLen, 1, &ulDummy));

                    // reading the module name
                    ENSURE_SAFE(DosRead(hfExe,
                                        paModules[i].achModuleName,
                                        bLen,
                                        &ulDummy));

                    // module names are not null terminated, so we must
                    // do it now
                    paModules[i].achModuleName[bLen] = 0;
                } // end for
            }
        } // end LX
        else if (pExec->ulExeFormat == EXEFORMAT_NE)
        {
            // 16-bit executable:
            cModules = pExec->pNEHeader->usModuleTblEntries;

            if (cModules)
            {
                ULONG cb = sizeof(FSYSMODULE) * cModules;

                paModules = (PFSYSMODULE)malloc(cb);
                if (!paModules)
                    ENSURE_FAIL(ERROR_NOT_ENOUGH_MEMORY);  // V0.9.9 (2001-04-03) [umoeller]

                memset(paModules, 0, cb);   // V0.9.9 (2001-04-03) [umoeller]

                for (i = 0; i < cModules; i ++)
                {
                    BYTE bLen;
                    USHORT usOfs;
                    ULONG ulDummy;

                    // the module reference table contains offsets
                    // relative to the import table; we hence read
                    // the offset in the module reference table, and
                    // then we read the name in the import table

                    ENSURE_SAFE(DosSetFilePtr(hfExe,
                                              pExec->pNEHeader->usModRefTblOfs
                                                + ulNewHeaderOfs // V0.9.12 (2001-05-03) [umoeller]
                                                + sizeof(usOfs) * i,
                                              FILE_BEGIN,
                                              &ulDummy));

                    ENSURE_SAFE(DosRead(hfExe, &usOfs, 2, &ulDummy));

                    ENSURE_SAFE(DosSetFilePtr(hfExe,
                                              pExec->pNEHeader->usImportTblOfs
                                                + ulNewHeaderOfs // V0.9.12 (2001-05-03) [umoeller]
                                                + usOfs,
                                              FILE_BEGIN,
                                              &ulDummy));

                    ENSURE_SAFE(DosRead(hfExe, &bLen, 1, &ulDummy));

                    ENSURE_SAFE(DosRead(hfExe,
                                        paModules[i].achModuleName,
                                        bLen,
                                        &ulDummy));

                    paModules[i].achModuleName[bLen] = 0;
                } // end for
            }
        } // end NE
        else
            ENSURE_FAIL(ERROR_INVALID_EXE_SIGNATURE); // V0.9.9 (2001-04-03) [umoeller]

        // no error: output data
        *ppaModules = paModules;
        *pcModules = cModules;

        ENSURE_FINALLY;
            // if we had an error above, clean up
            free(paModules);
        ENSURE_END;
    }
    else
        ENSURE_FAIL(ERROR_INVALID_EXE_SIGNATURE); // V0.9.9 (2001-04-03) [umoeller]

    ENSURE_OK;
}

/*
 *@@ exehFreeImportedModules:
 *      frees resources allocated by exehQueryImportedModules.
 *
 *@@added V0.9.9 (2001-03-11)
 */

APIRET exehFreeImportedModules(PFSYSMODULE paModules)
{
    free(paModules);
    return NO_ERROR;
}

/*
 *@@ ScanLXEntryTable:
 *      returns the number of exported entries in the entry table.
 *
 *      If paFunctions is not NULL, then successive entries are
 *      filled with the found type and ordinal values.
 *
 *@@added V0.9.9 (2001-03-30) [lafaix]
 *@@changed V0.9.9 (2001-04-03) [umoeller]: added tons of error checking, changed prototype to return APIRET
 *@@changed V0.9.9 (2001-04-05) [lafaix]: rewritten error checking code
 *@@changed V0.9.12 (2001-05-03) [umoeller]: adjusted for new NOSTUB support
 */

STATIC APIRET ScanLXEntryTable(PEXECUTABLE pExec,
                               PFSYSFUNCTION paFunctions,
                               PULONG pcEntries)        // out: entry table entry count; ptr can be NULL
{
    ULONG  ulDummy;
    USHORT usOrdinal = 1,
           usCurrent = 0;
    int    i;

    ULONG ulNewHeaderOfs = 0; // V0.9.12 (2001-05-03) [umoeller]
    HFILE hfExe = pExec->pFile->hf;

    if (pExec->cbDosExeHeader)
        // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
        ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

    ENSURE(DosSetFilePtr(hfExe,
                         pExec->pLXHeader->ulEntryTblOfs
                           + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                         FILE_BEGIN,
                         &ulDummy));

    while (TRUE)
    {
        BYTE   bCnt,
               bType,
               bFlag;

        ENSURE(DosRead(hfExe, &bCnt, 1, &ulDummy));

        if (bCnt == 0)
            // end of the entry table
            break;

        ENSURE(DosRead(hfExe, &bType, 1, &ulDummy));

        switch (bType & 0x7F)
        {
            /*
             * unused entries
             *
             */

            case 0:
                usOrdinal += bCnt;
            break;

            /*
             * 16-bit entries
             *
             * the bundle type is followed by the object number
             * and by bCnt bFlag+usOffset entries
             *
             */

            case 1:
                ENSURE(DosSetFilePtr(hfExe,
                                     sizeof(USHORT),
                                     FILE_CURRENT,
                                     &ulDummy));

                for (i = 0; i < bCnt; i ++)
                {
                    ENSURE(DosRead(hfExe, &bFlag, 1, &ulDummy));

                    if (bFlag & 0x01)
                    {
                        if (paFunctions)
                        {
                            paFunctions[usCurrent].ulOrdinal = usOrdinal;
                            paFunctions[usCurrent].ulType = 1;
                            paFunctions[usCurrent].achFunctionName[0] = 0;
                        }
                        usCurrent++;
                    }

                    usOrdinal++;

                    ENSURE(DosSetFilePtr(hfExe,
                                         sizeof(USHORT),
                                         FILE_CURRENT,
                                         &ulDummy));

                } // end for
            break;

            /*
             * 286 call gate entries
             *
             * the bundle type is followed by the object number
             * and by bCnt bFlag+usOffset+usCallGate entries
             *
             */

            case 2:
                ENSURE(DosSetFilePtr(hfExe,
                                     sizeof(USHORT),
                                     FILE_CURRENT,
                                     &ulDummy));

                for (i = 0; i < bCnt; i ++)
                {
                    ENSURE(DosRead(hfExe, &bFlag, 1, &ulDummy));

                    if (bFlag & 0x01)
                    {
                        if (paFunctions)
                        {
                            paFunctions[usCurrent].ulOrdinal = usOrdinal;
                            paFunctions[usCurrent].ulType = 2;
                            paFunctions[usCurrent].achFunctionName[0] = 0;
                        }
                        usCurrent++;
                    }

                    usOrdinal++;

                    ENSURE(DosSetFilePtr(hfExe,
                                         sizeof(USHORT) + sizeof(USHORT),
                                         FILE_CURRENT,
                                         &ulDummy));

                } // end for
            break;

            /*
             * 32-bit entries
             *
             * the bundle type is followed by the object number
             * and by bCnt bFlag+ulOffset entries
             *
             */

            case 3:
                ENSURE(DosSetFilePtr(hfExe,
                                     sizeof(USHORT),
                                     FILE_CURRENT,
                                     &ulDummy));

                for (i = 0; i < bCnt; i ++)
                {
                    ENSURE(DosRead(hfExe, &bFlag, 1, &ulDummy));

                    if (bFlag & 0x01)
                    {
                        if (paFunctions)
                        {
                            paFunctions[usCurrent].ulOrdinal = usOrdinal;
                            paFunctions[usCurrent].ulType = 3;
                            paFunctions[usCurrent].achFunctionName[0] = 0;
                        }
                        usCurrent++;
                    }

                    usOrdinal++;

                    ENSURE(DosSetFilePtr(hfExe,
                                         sizeof(ULONG),
                                         FILE_CURRENT,
                                         &ulDummy));
                } // end for
            break;

            /*
             * forwarder entries
             *
             * the bundle type is followed by a reserved word
             * and by bCnt bFlag+usModOrd+ulOffsOrdNum entries
             *
             */

            case 4:
                ENSURE(DosSetFilePtr(hfExe,
                                     sizeof(USHORT),
                                     FILE_CURRENT,
                                     &ulDummy));

                for (i = 0; i < bCnt; i ++)
                {
                    ENSURE(DosSetFilePtr(hfExe,
                                         sizeof(BYTE) + sizeof(USHORT) + sizeof(ULONG),
                                         FILE_CURRENT,
                                         &ulDummy));

                    if (paFunctions)
                    {
                        paFunctions[usCurrent].ulOrdinal = usOrdinal;
                        paFunctions[usCurrent].ulType = 4;
                        paFunctions[usCurrent].achFunctionName[0] = 0;
                    }
                    usCurrent++;

                    usOrdinal++;
                } // end for
            break;

            /*
             * unknown bundle type
             *
             * we don't know how to handle this bundle, so we must
             * stop parsing the entry table here (as we don't know the
             * bundle size); if paFunctions is not null, we fill it with
             * informative data
             */

            default:
                if (paFunctions)
                {
                    paFunctions[usCurrent].ulOrdinal = usOrdinal;
                    paFunctions[usCurrent].ulType = bType;
                    sprintf(paFunctions[usCurrent].achFunctionName,
                            "Unknown bundle type encountered (%d).  Aborting entry table scan.",
                            bType);

                    usCurrent++;
                }
                ENSURE_FAIL(ERROR_INVALID_LIST_FORMAT);
                    // whatever
                    // V0.9.9 (2001-04-03) [umoeller]
        } // end switch (bType & 0x7F)
    } // end while (TRUE)

    if (pcEntries)
       *pcEntries = usCurrent;

    ENSURE_OK;
}

/*
 *@@ ScanNEEntryTable:
 *      returns the number of exported entries in the entry table.
 *
 *      if paFunctions is not NULL, then successive entries are
 *      filled with the found type and ordinal values.
 *
 *@@added V0.9.9 (2001-03-30) [lafaix]
 *@@changed V0.9.9 (2001-04-03) [umoeller]: added tons of error checking, changed prototype to return APIRET
 *@@changed V0.9.9 (2001-04-05) [lafaix]: rewritten error checking code
 *@@changed V0.9.12 (2001-05-03) [umoeller]: adjusted for new NOSTUB support
 */

STATIC APIRET ScanNEEntryTable(PEXECUTABLE pExec,
                               PFSYSFUNCTION paFunctions,
                               PULONG pcEntries)        // out: entry table entry count; ptr can be NULL
{
    ULONG  ulDummy;
    USHORT usOrdinal = 1,
           usCurrent = 0;
    int    i;

    ULONG ulNewHeaderOfs = 0;
    HFILE hfExe = pExec->pFile->hf;

    if (pExec->cbDosExeHeader)
        // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
        ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

    ENSURE(DosSetFilePtr(hfExe,
                         pExec->pNEHeader->usEntryTblOfs
                           + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                         FILE_BEGIN,
                         &ulDummy));

    while (TRUE)
    {
        BYTE bCnt,
             bType,
             bFlag;

        ENSURE(DosRead(hfExe, &bCnt, 1, &ulDummy));

        if (bCnt == 0)
            // end of the entry table
            break;

        ENSURE(DosRead(hfExe, &bType, 1, &ulDummy));

        if (bType)
        {
            for (i = 0; i < bCnt; i++)
            {
                ENSURE(DosRead(hfExe,
                               &bFlag,
                               1,
                               &ulDummy));

                if (bFlag & 0x01)
                {
                    if (paFunctions)
                    {
                        paFunctions[usCurrent].ulOrdinal = usOrdinal;
                        paFunctions[usCurrent].ulType = 1; // 16-bit entry
                        paFunctions[usCurrent].achFunctionName[0] = 0;
                    }
                    usCurrent++;
                }

                usOrdinal++;

                if (bType == 0xFF)
                {
                    // moveable segment
                    ENSURE(DosSetFilePtr(hfExe,
                                         5,
                                         FILE_CURRENT,
                                         &ulDummy));
                }
                else
                {
                    // fixed segment or constant (0xFE)
                    ENSURE(DosSetFilePtr(hfExe,
                                         2,
                                         FILE_CURRENT,
                                         &ulDummy));
                }

            } // end for
        }
        else
            usOrdinal += bCnt;
    } // end while (TRUE)

    if (pcEntries)
        *pcEntries = usCurrent;

    ENSURE_OK;
}

/*
 *@@ Compare:
 *      binary search helper
 *
 *@@added V0.9.9 (2001-04-01) [lafaix]
 *@@changed V0.9.9 (2001-04-07) [umoeller]: added _Optlink, or this won't compile as C++
 */

STATIC int /*_Optlink*/ Compare(const void *key,
                            const void *element)
{
    USHORT        usOrdinal = *((PUSHORT) key);
    PFSYSFUNCTION pFunction = (PFSYSFUNCTION)element;

    if (usOrdinal > pFunction->ulOrdinal)
        return 1;
    else if (usOrdinal < pFunction->ulOrdinal)
        return -1;

    return 0;
}

/*
 *@@ ScanNameTable:
 *      scans a resident or non-resident name table, and fills the
 *      appropriate paFunctions entries when it encounters exported
 *      entries names.
 *
 *      This functions works for both NE and LX executables.
 *
 *@@added V0.9.9 (2001-03-30) [lafaix]
 *@@changed V0.9.9 (2001-04-02) [lafaix]: the first entry is special
 *@@changed V0.9.9 (2001-04-03) [umoeller]: added tons of error checking, changed prototype to return APIRET
 *@@changed V0.9.9 (2001-04-05) [lafaix]: removed the 127 char limit
 *@@changed V0.9.9 (2001-04-05) [lafaix]: rewritten error checking code
 */

STATIC APIRET ScanNameTable(PEXECUTABLE pExec,
                            ULONG cFunctions,
                            PFSYSFUNCTION paFunctions)
{
    ULONG   ulDummy;

    USHORT        usOrdinal;
    PFSYSFUNCTION pFunction;
    HFILE hfExe = pExec->pFile->hf;

    while (TRUE)
    {
        BYTE   bLen;
        CHAR   achName[256];
        // int    i;

        ENSURE(DosRead(hfExe, &bLen, 1, &ulDummy));

        if (bLen == 0)
            // end of the name table
            break;

        ENSURE(DosRead(hfExe, &achName, bLen, &ulDummy));
        achName[bLen] = 0;

        ENSURE(DosRead(hfExe, &usOrdinal, sizeof(USHORT), &ulDummy));

        if ((pFunction = (PFSYSFUNCTION)bsearch(&usOrdinal,
                                                paFunctions,
                                                cFunctions,
                                                sizeof(FSYSFUNCTION),
                                                Compare)))
        {
            memcpy(pFunction->achFunctionName,
                   achName,
                   bLen+1);
        }
    }

    ENSURE_OK;
}

/*
 *@@ exehQueryExportedFunctions:
 *      returns an array of FSYSFUNCTION structure describing all
 *      exported functions.
 *
 *      *pcFunctions receives the # of items in the array (not the
 *      array size!).  Use doshFreeExportedFunctions to clean up.
 *
 *      Note that the returned array only contains entry for exported
 *      functions.  Empty export entries are _not_ included.
 *
 *      This returns a standard OS/2 error code, which might be
 *      any of the codes returned by DosSetFilePtr and DosRead.
 *      In addition, this may return:
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: exe is in a format other
 *          than LX or NE, which is not understood by this function.
 *
 *      --  If ERROR_INVALID_LIST_FORMAT is returned, the format of an
 *          export entry wasn't understood here.
 *
 *      Even if NO_ERROR is returned, the array pointer might still
 *      be NULL if the module contains no such data.
 *
 *@@added V0.9.9 (2001-03-11) [lafaix]
 *@@changed V0.9.9 (2001-04-03) [umoeller]: added tons of error checking, changed prototype to return APIRET
 *@@changed V0.9.9 (2001-04-05) [lafaix]: rewritten error checking code
 *@@changed V0.9.10 (2001-04-10) [lafaix]: added Win16 and Win386 support
 *@@changed V0.9.12 (2001-05-03) [umoeller]: adjusted for new NOSTUB support
 */

APIRET exehQueryExportedFunctions(PEXECUTABLE pExec,
                                  PFSYSFUNCTION *ppaFunctions,  // out: functions array
                                  PULONG pcFunctions)           // out: array item count
{
    if (    (pExec)
         && (    (pExec->ulOS == EXEOS_OS2)
              || (pExec->ulOS == EXEOS_WIN16)
              || (pExec->ulOS == EXEOS_WIN386)
            )
       )
    {
        ENSURE_BEGIN;
        ULONG         cFunctions = 0;
        PFSYSFUNCTION paFunctions = NULL;

        ULONG ulDummy;

        HFILE hfExe = pExec->pFile->hf;
        ULONG ulNewHeaderOfs = 0; // V0.9.12 (2001-05-03) [umoeller]

        if (pExec->cbDosExeHeader)
            // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
            ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

        if (pExec->ulExeFormat == EXEFORMAT_LX)
        {
            // It's a 32bit OS/2 executable

            // the number of exported entry points is not stored
            // in the executable header; we have to count them in
            // the entry table

            ENSURE(ScanLXEntryTable(pExec, NULL, &cFunctions));

            // we now have the number of exported entries; let us
            // build them

            if (cFunctions)
            {
                ULONG cb = sizeof(FSYSFUNCTION) * cFunctions;

                paFunctions = (PFSYSFUNCTION)malloc(cb);
                if (!paFunctions)
                    ENSURE_FAIL(ERROR_NOT_ENOUGH_MEMORY);

                // we rescan the entry table (the cost is not as bad
                // as it may seem, due to disk caching)

                ENSURE_SAFE(ScanLXEntryTable(pExec, paFunctions, NULL));

                // we now scan the resident name table entries

                ENSURE_SAFE(DosSetFilePtr(hfExe,
                                          pExec->pLXHeader->ulResdNameTblOfs
                                            + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                                          FILE_BEGIN,
                                          &ulDummy));

                ENSURE_SAFE(ScanNameTable(pExec, cFunctions, paFunctions));

                // we now scan the non-resident name table entries,
                // whose offset is _from the begining of the file_

                ENSURE_SAFE(DosSetFilePtr(hfExe,
                                          pExec->pLXHeader->ulNonResdNameTblOfs,
                                          FILE_BEGIN,
                                          &ulDummy));

                ENSURE_SAFE(ScanNameTable(pExec, cFunctions, paFunctions));
            } // end if (cFunctions)
        }
        else if (pExec->ulExeFormat == EXEFORMAT_NE)
        {
            // it's a "new" segmented 16bit executable

            // here too the number of exported entry points
            // is not stored in the executable header; we
            // have to count them in the entry table

            ENSURE(ScanNEEntryTable(pExec, NULL, &cFunctions));

            // we now have the number of exported entries; let us
            // build them

            if (cFunctions)
            {
                // USHORT usOrdinal = 1;
                       // usCurrent = 0;

                paFunctions = (PFSYSFUNCTION)malloc(sizeof(FSYSFUNCTION) * cFunctions);
                if (!paFunctions)
                    ENSURE_FAIL(ERROR_NOT_ENOUGH_MEMORY);

                // we rescan the entry table (the cost is not as bad
                // as it may seem, due to disk caching)

                ENSURE_SAFE(ScanNEEntryTable(pExec, paFunctions, NULL));

                // we now scan the resident name table entries

                ENSURE_SAFE(DosSetFilePtr(hfExe,
                                          pExec->pNEHeader->usResdNameTblOfs
                                            + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                                          FILE_BEGIN,
                                          &ulDummy));

                ENSURE_SAFE(ScanNameTable(pExec, cFunctions, paFunctions));

                // we now scan the non-resident name table entries,
                // whose offset is _from the begining of the file_

                ENSURE_SAFE(DosSetFilePtr(hfExe,
                                          pExec->pNEHeader->ulNonResdTblOfs,
                                          FILE_BEGIN,
                                          &ulDummy));

                ENSURE_SAFE(ScanNameTable(pExec, cFunctions, paFunctions));
            }
        }
        else
            ENSURE_FAIL(ERROR_INVALID_EXE_SIGNATURE); // V0.9.9 (2001-04-03) [umoeller]

        // no error: output data
        *ppaFunctions = paFunctions;
        *pcFunctions = cFunctions;

        ENSURE_FINALLY;
            // if we had an error above, clean up
            free(paFunctions);
        ENSURE_END;
    }
    else
        ENSURE_FAIL(ERROR_INVALID_EXE_SIGNATURE); // V0.9.9 (2001-04-03) [umoeller]

    ENSURE_OK;
}

/*
 *@@ exehFreeExportedFunctions:
 *      frees resources allocated by exehQueryExportedFunctions.
 *
 *@@added V0.9.9 (2001-03-11)
 */

APIRET exehFreeExportedFunctions(PFSYSFUNCTION paFunctions)
{
    free(paFunctions);

    return NO_ERROR;
}

/*
 *@@ exehQueryResources:
 *      returns an array of FSYSRESOURCE structures describing all
 *      available resources in the module.
 *
 *      *pcResources receives the no. of items in the array
 *      (not the array size!). Use exehFreeResources to clean up.
 *
 *      This returns a standard OS/2 error code, which might be
 *      any of the codes returned by DosSetFilePtr and DosRead.
 *      In addition, this may return:
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: exe is in a format other
 *          than LX or NE, which is not understood by this function.
 *
 *      Even if NO_ERROR is returned, the array pointer might still
 *      be NULL if the module contains no such data.
 *
 *@@added V0.9.7 (2000-12-18) [lafaix]
 *@@changed V0.9.9 (2001-04-03) [umoeller]: added tons of error checking, changed prototype to return APIRET
 *@@changed V0.9.10 (2001-04-10) [lafaix]: added Win16 and Win386 support
 *@@changed V0.9.12 (2001-05-03) [umoeller]: adjusted for new NOSTUB support
 */

APIRET exehQueryResources(PEXECUTABLE pExec,     // in: executable from exehOpen
                          PFSYSRESOURCE *ppaResources,   // out: res's array
                          PULONG pcResources)    // out: array item count
{
    if (    (pExec)
         && (    (pExec->ulOS == EXEOS_OS2)
              || (pExec->ulOS == EXEOS_WIN16)
              || (pExec->ulOS == EXEOS_WIN386)
            )
       )
    {
        ENSURE_BEGIN;
        ULONG           cResources = 0;
        PFSYSRESOURCE   paResources = NULL;

        HFILE hfExe = pExec->pFile->hf;
        ULONG           ulNewHeaderOfs = 0; // V0.9.12 (2001-05-03) [umoeller]

        if (pExec->cbDosExeHeader)
            // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
            ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

        if (pExec->ulExeFormat == EXEFORMAT_LX)
        {
            // 32-bit OS/2 executable:
            PLXHEADER pLXHeader = pExec->pLXHeader;
            if (cResources = pLXHeader->ulResTblCnt)
            {
                #pragma pack(1)     // V0.9.9 (2001-04-02) [umoeller]
                struct rsrc32               // Resource Table Entry
                {
                    unsigned short  type;   // Resource type
                    unsigned short  name;   // Resource name
                    unsigned long   cb;     // Resource size
                    unsigned short  obj;    // Object number
                    unsigned long   offset; // Offset within object
                } rs;

                struct o32_obj                    // Flat .EXE object table entry
                {
                    unsigned long   o32_size;     // Object virtual size
                    unsigned long   o32_base;     // Object base virtual address
                    unsigned long   o32_flags;    // Attribute flags
                    unsigned long   o32_pagemap;  // Object page map index
                    unsigned long   o32_mapsize;  // Number of entries in object page map
                    unsigned long   o32_reserved; // Reserved
                } ot;
                #pragma pack() // V0.9.9 (2001-04-03) [umoeller]

                ULONG cb = sizeof(FSYSRESOURCE) * cResources;
                ULONG ulDummy;
                int i;
                ULONG ulCurOfs;

                paResources = (PFSYSRESOURCE)malloc(cb);
                if (!paResources)
                    ENSURE_FAIL(ERROR_NOT_ENOUGH_MEMORY);

                memset(paResources, 0, cb); // V0.9.9 (2001-04-03) [umoeller]

                ENSURE_SAFE(DosSetFilePtr(hfExe,
                                          pLXHeader->ulResTblOfs
                                            + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                                          FILE_BEGIN,
                                          &ulDummy));

                for (i = 0; i < cResources; i++)
                {
                    ENSURE_SAFE(DosRead(hfExe, &rs, 14, &ulDummy));

                    paResources[i].ulID = rs.name;
                    paResources[i].ulType = rs.type;
                    paResources[i].ulSize = rs.cb;
                    paResources[i].ulFlag = rs.obj; // Temp storage for Object
                                                    // number.  Will be filled
                                                    // with resource flag
                                                    // later.
                }

                for (i = 0; i < cResources; i++)
                {
                    ULONG ulOfsThis =   pLXHeader->ulObjTblOfs
                                      + ulNewHeaderOfs // V0.9.12 (2001-05-03) [umoeller]
                                      + (   sizeof(ot)
                                          * (paResources[i].ulFlag - 1));

                    ENSURE_SAFE(DosSetFilePtr(hfExe,
                                              ulOfsThis,
                                              FILE_BEGIN,
                                              &ulDummy));

                    ENSURE_SAFE(DosRead(hfExe, &ot, sizeof(ot), &ulDummy));

                    paResources[i].ulFlag  = ((ot.o32_flags & OBJWRITE)
                                                    ? 0
                                                    : RNPURE);
                    paResources[i].ulFlag |= ((ot.o32_flags & OBJDISCARD)
                                                    ? 4096
                                                    : 0);
                    paResources[i].ulFlag |= ((ot.o32_flags & OBJSHARED)
                                                    ? RNMOVE
                                                    : 0);
                    paResources[i].ulFlag |= ((ot.o32_flags & OBJPRELOAD)
                                                    ? RNPRELOAD
                                                    : 0);
                } // end for
            } // end if (cResources)
        } // end if (pExec->ulExeFormat == EXEFORMAT_LX)
        else if (pExec->ulExeFormat == EXEFORMAT_NE)
        {
            PNEHEADER pNEHeader = pExec->pNEHeader;

            if (pExec->ulOS == EXEOS_OS2)
            {
                // 16-bit OS/2 executable:
                cResources = pNEHeader->usResSegmCount;

                if (cResources)
                {
                    #pragma pack(1)     // V0.9.9 (2001-04-02) [umoeller]
                    struct {unsigned short type; unsigned short name;} rti;
                    struct new_seg                          // New .EXE segment table entry
                    {
                        unsigned short      ns_sector;      // File sector of start of segment
                        unsigned short      ns_cbseg;       // Number of bytes in file
                        unsigned short      ns_flags;       // Attribute flags
                        unsigned short      ns_minalloc;    // Minimum allocation in bytes
                    } ns;
                    #pragma pack()

                    ULONG cb = sizeof(FSYSRESOURCE) * cResources;
                    ULONG ulDummy;
                    int i;

                    paResources = (PFSYSRESOURCE)malloc(cb);
                    if (!paResources)
                        ENSURE_FAIL(ERROR_NOT_ENOUGH_MEMORY);

                    memset(paResources, 0, cb);     // V0.9.9 (2001-04-03) [umoeller]

                    // we first read the resources IDs and types

                    ENSURE_SAFE(DosSetFilePtr(hfExe,
                                              pNEHeader->usResTblOfs
                                                + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                                              FILE_BEGIN,
                                              &ulDummy));

                    for (i = 0; i < cResources; i++)
                    {
                        ENSURE_SAFE(DosRead(hfExe, &rti, sizeof(rti), &ulDummy));

                        paResources[i].ulID = rti.name;
                        paResources[i].ulType = rti.type;
                    }

                    // we then read their sizes and flags

                    for (i = 0; i < cResources; i++)
                    {
                        ENSURE_SAFE(DosSetFilePtr(hfExe,
                                                  ulNewHeaderOfs // V0.9.12 (2001-05-03) [umoeller]
                                                    + pNEHeader->usSegTblOfs
                                                    + (sizeof(ns)
                                                    * (  pNEHeader->usSegTblEntries
                                                       - pNEHeader->usResSegmCount
                                                       + i)),
                                                    FILE_BEGIN,
                                                    &ulDummy));

                        ENSURE_SAFE(DosRead(hfExe, &ns, sizeof(ns), &ulDummy));

                        paResources[i].ulSize = ns.ns_cbseg;

                        paResources[i].ulFlag  = (ns.ns_flags & OBJPRELOAD) ? RNPRELOAD : 0;
                        paResources[i].ulFlag |= (ns.ns_flags & OBJSHARED) ? RNPURE : 0;
                        paResources[i].ulFlag |= (ns.ns_flags & OBJDISCARD) ? RNMOVE : 0;
                        paResources[i].ulFlag |= (ns.ns_flags & OBJDISCARD) ? 4096 : 0;
                    }
                } // end if (cResources)
            }
            else
            {
                // 16-bit Windows executable
                USHORT usAlignShift;
                ULONG  ulDummy;

                ENSURE(DosSetFilePtr(hfExe,
                                     pNEHeader->usResTblOfs
                                       + ulNewHeaderOfs, // V0.9.12 (2001-05-03) [umoeller]
                                     FILE_BEGIN,
                                     &ulDummy));

                ENSURE(DosRead(hfExe,
                               &usAlignShift,
                               sizeof(usAlignShift),
                               &ulDummy));

                while (TRUE)
                {
                    USHORT usTypeID;
                    USHORT usCount;

                    ENSURE(DosRead(hfExe,
                                   &usTypeID,
                                   sizeof(usTypeID),
                                   &ulDummy));

                    if (usTypeID == 0)
                        break;

                    ENSURE(DosRead(hfExe,
                                   &usCount,
                                   sizeof(usCount),
                                   &ulDummy));

                    ENSURE(DosSetFilePtr(hfExe,
                                         sizeof(ULONG),
                                         FILE_CURRENT,
                                         &ulDummy));

                    cResources += usCount;

                    // first pass, skip NAMEINFO table
                    ENSURE(DosSetFilePtr(hfExe,
                                         usCount*6*sizeof(USHORT),
                                         FILE_CURRENT,
                                         &ulDummy));
                }

                if (cResources)
                {
                    USHORT usCurrent = 0;
                    ULONG cb = sizeof(FSYSRESOURCE) * cResources;

                    paResources = (PFSYSRESOURCE)malloc(cb);
                    if (!paResources)
                        ENSURE_FAIL(ERROR_NOT_ENOUGH_MEMORY);

                    memset(paResources, 0, cb);

                    ENSURE_SAFE(DosSetFilePtr(hfExe,
                                              pNEHeader->usResTblOfs
                                                + ulNewHeaderOfs,
                                              FILE_BEGIN,
                                              &ulDummy));

                    ENSURE_SAFE(DosRead(hfExe,
                                        &usAlignShift,
                                        sizeof(usAlignShift),
                                        &ulDummy));

                    while (TRUE)
                    {
                        USHORT usTypeID;
                        USHORT usCount;
                        int i;

                        ENSURE_SAFE(DosRead(hfExe,
                                            &usTypeID,
                                            sizeof(usTypeID),
                                            &ulDummy));

                        if (usTypeID == 0)
                            break;

                        ENSURE_SAFE(DosRead(hfExe,
                                            &usCount,
                                            sizeof(usCount),
                                            &ulDummy));

                        ENSURE_SAFE(DosSetFilePtr(hfExe,
                                                  sizeof(ULONG),
                                                  FILE_CURRENT,
                                                  &ulDummy));

                        // second pass, read NAMEINFO table
                        for (i = 0; i < usCount; i++)
                        {
                            USHORT usLength,
                                   usFlags,
                                   usID;

                            ENSURE_SAFE(DosSetFilePtr(hfExe,
                                                      sizeof(USHORT),
                                                      FILE_CURRENT,
                                                      &ulDummy));

                            ENSURE_SAFE(DosRead(hfExe,
                                                &usLength,
                                                sizeof(USHORT),
                                                &ulDummy));
                            ENSURE_SAFE(DosRead(hfExe,
                                                &usFlags,
                                                sizeof(USHORT),
                                                &ulDummy));
                            ENSURE_SAFE(DosRead(hfExe,
                                                &usID,
                                                sizeof(USHORT),
                                                &ulDummy));

                            ENSURE_SAFE(DosSetFilePtr(hfExe,
                                                      2*sizeof(USHORT),
                                                      FILE_CURRENT,
                                                      &ulDummy));

                            // !!! strings ids and types not handled yet
                            // !!! 15th bit is used to denotes strings
                            // !!! offsets [lafaix]
                            paResources[usCurrent].ulType = usTypeID ^ 0x8000;
                            paResources[usCurrent].ulID = usID ^ 0x8000;
                            paResources[usCurrent].ulSize = usLength << usAlignShift;
                            paResources[usCurrent].ulFlag = usFlags & 0x70;

                            usCurrent++;
                        }
                    }
                }
            }
        } // end else if (pExec->ulExeFormat == EXEFORMAT_NE)
        else
            ENSURE_FAIL(ERROR_INVALID_EXE_SIGNATURE); // V0.9.9 (2001-04-03) [umoeller]

        *ppaResources = paResources;
        *pcResources = cResources;

        ENSURE_FINALLY;
            // if we had an error above, clean up
            free(paResources);
        ENSURE_END;
    }
    else
        ENSURE_FAIL(ERROR_INVALID_EXE_SIGNATURE); // V0.9.9 (2001-04-03) [umoeller]

    ENSURE_OK;
}

/*
 *@@ exehFreeResources:
 *      frees resources allocated by exehQueryResources.
 *
 *@@added V0.9.7 (2000-12-18) [lafaix]
 */

APIRET exehFreeResources(PFSYSRESOURCE paResources)
{
    free(paResources);
    return NO_ERROR;
}

/*
 *@@ exehLoadLXMaps:
 *      loads the three main LX maps into the given
 *      EXECUTABLE structure.
 *
 *      This loads:
 *
 *      1)  the LX resource table;
 *
 *      2)  the LX object table;
 *
 *      3)  the LX object _page_ table (object map).
 *
 *      Note that this is not automatically called
 *      by exehOpen to save time, since the LX
 *      maps are not needed for all the other exe
 *      functions. However, this does get called
 *      from exehLoadLXResource if needed.
 *
 *      This returns:
 *
 *      --  NO_ERROR: all three LX maps were loaded,
 *          and pExec->fLXMapsLoaded was set to TRUE.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: pExec does
 *          not specify an LX executable.
 *
 *      --  ERROR_NO_DATA: at least one of the structs
 *          does not exist.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      plus the error codes of doshReadAt.
 *
 *      Call exehFreeLXMaps to clean up explicitly, but
 *      that func automatically gets called by exehClose.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

APIRET exehLoadLXMaps(PEXECUTABLE pExec)
{
    APIRET arc;

    PLXHEADER pLXHeader;

    if (!pExec)
        arc = ERROR_INVALID_PARAMETER;
    else if (pExec->fLXMapsLoaded)
        // already loaded:
        arc = NO_ERROR;
    else if (    (pExec->ulExeFormat != EXEFORMAT_LX)
              || (!(pLXHeader = pExec->pLXHeader))
            )
        arc = ERROR_INVALID_EXE_SIGNATURE;
    else
    {
        PXFILE pFile = pExec->pFile;
        ULONG ulNewHeaderOfs = 0;
        ULONG cb;

        if (pExec->cbDosExeHeader)
            // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
            ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

        // resource table
        if (    (!(arc = doshAllocArray(pLXHeader->ulResTblCnt,
                                        sizeof(RESOURCETABLEENTRY),
                                        (PBYTE*)&pExec->pRsTbl,
                                        &cb)))
             && (!(arc = doshReadAt(pFile,
                                    pLXHeader->ulResTblOfs
                                      + ulNewHeaderOfs,
                                    &cb,
                                    (PBYTE)pExec->pRsTbl,
                                    DRFL_FAILIFLESS)))
            )
        {
            // object table
            if (    (!(arc = doshAllocArray(pLXHeader->ulObjCount,
                                            sizeof(OBJECTTABLEENTRY),
                                            (PBYTE*)&pExec->pObjTbl,
                                            &cb)))
                 && (!(arc = doshReadAt(pFile,
                                        pLXHeader->ulObjTblOfs
                                          + ulNewHeaderOfs,
                                        &cb,
                                        (PBYTE)pExec->pObjTbl,
                                        DRFL_FAILIFLESS)))
               )
            {
                // object page table
                if (    (!(arc = doshAllocArray(pLXHeader->ulPageCount,
                                                sizeof(OBJECTPAGETABLEENTRY),
                                                (PBYTE*)&pExec->pObjPageTbl,
                                                &cb)))
                     && (!(arc = doshReadAt(pFile,
                                            pLXHeader->ulObjPageTblOfs
                                              + ulNewHeaderOfs,
                                            &cb,
                                            (PBYTE)pExec->pObjPageTbl,
                                            DRFL_FAILIFLESS)))
                   )
                {
                }
            }
        }

        if (!arc)
            pExec->fLXMapsLoaded = TRUE;
        else
            exehFreeLXMaps(pExec);
    }

    return arc;
}

/*
 *@@ exehFreeLXMaps:
 *      frees data allocated by exehLoadLXMaps.
 *      Gets called automatically by exehClose.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

VOID exehFreeLXMaps(PEXECUTABLE pExec)
{
    FREE(pExec->pRsTbl);
    FREE(pExec->pObjTbl);
    FREE(pExec->pObjPageTbl);
    pExec->fLXMapsLoaded = FALSE;
}

// page flags (OBJECTPAGETABLEENTRY.o32_pageflags)
#define VALID           0x0000                // Valid Physical Page in .EXE
#define ITERDATA        0x0001                // Iterated Data Page
#define INVALID         0x0002                // Invalid Page
#define ZEROED          0x0003                // Zero Filled Page
#define RANGE           0x0004                // Range of pages
#define ITERDATA2       0x0005                // Iterated Data Page Type II

/*
 *@@ ExpandIterdata1:
 *      expands a page compressed with the old exepack
 *      method introduced with OS/2 2.0 (plain /EXEPACK).
 *
 *      Returns either ERROR_BAD_FORMAT or NO_ERROR.
 *
 *      (C) Knut Stange Osmundsen. Used with permission.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC APIRET ExpandIterdata1(char *pabTarget,         // out: page data (pagesize as in lx spec)
                              int cbTarget,            // in: sizeof *pabTarget (pagesize as in lx spec)
                              PCSZ pabSource,   // in: compressed source data in EXEPACK:1 format
                              int cbSource)            // in: sizeof *pabSource
{
    PLXITER             pIter = (PLXITER)pabSource;
    // store the pointer for boundary checking
    char                *pabTargetOriginal = pabTarget;

    // validate size of data
    if (cbSource >= cbTarget - 2)
        return ERROR_BAD_FORMAT;

    // expand the page
    while (    (pIter->LX_nIter)
            && (cbSource > 0)
          )
    {
        // check if we're out of bound
        ULONG nIter = pIter->LX_nIter,
              nBytes = pIter->LX_nBytes;

        if (    (pabTarget - pabTargetOriginal + nIter * nBytes > cbTarget)
             || (cbSource <= 0)
           )
            return ERROR_BAD_FORMAT;

        if (nBytes == 1)
        {
            // one databyte
            memset(pabTarget, pIter->LX_Iterdata, nIter);
            pabTarget += nIter;
            cbSource -= 4 + 1;
            pIter++;
        }
        else
        {
            int i;
            for (i = nIter;
                 i > 0;
                 i--, pabTarget += nBytes)
                memcpy(pabTarget, &pIter->LX_Iterdata, nBytes);
            cbSource -= 4 + nBytes;
            pIter   = (PLXITER)((char*)pIter + 4 + nBytes);
        }
    }

    // zero remaining part of the page
    if (pabTarget - pabTargetOriginal < cbTarget)
        memset(pabTarget, 0, cbTarget - (pabTarget - pabTargetOriginal));

    return NO_ERROR;
}

/*
 *@@ memcpyw:
 *      a special memcpy for expandPage2 which performs a
 *      word based copy. The difference between this, memmove
 *      and memcpy is that we'll allways read words.
 *
 *      (C) Knut Stange Osmundsen. Used with permission.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC void memcpyw(char *pch1, PCSZ pch2, size_t cch)
{
    /*
     * Use memcpy if possible.
     */
    if ((pch2 > pch1 ? pch2 - pch1 : pch1 - pch2) >= 4)
    {
        memcpy(pch1, pch2, cch);        /* BUGBUG! ASSUMES that memcpy move NO more than 4 bytes at the time! */
        return;
    }

    /*
     * Difference is less than 3 bytes.
     */
    if (cch & 1)
        *pch1++ = *pch2++;

    for (cch >>= 1;
         cch > 0;
         cch--, pch1 += 2, pch2 += 2)
        *(PUSHORT)pch1 = *(PUSHORT)pch2;
}

/*
 *@@ memcpyb:
 *      a special memcpy for expandPage2 which performs a memmove
 *      operation. The difference between this and memmove is that
 *      this one works.
 *
 *      (C) Knut Stange Osmundsen. Used with permission.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC void memcpyb(char *pch1, PCSZ pch2, size_t cch)
{
    /*
     * Use memcpy if possible.
     */
    if ((pch2 > pch1 ? pch2 - pch1 : pch1 - pch2) >= 4)
    {
        memcpy(pch1, pch2, cch);
        return;
    }

    /*
     * Difference is less than 3 bytes.
     */
    while(cch--)
        *pch1++ = *pch2++;
}

/*
 *@@ ExpandIterdata2:
 *      expands a page compressed with the new exepack
 *      method introduced with OS/2 Warp 3.0 (/EXEPACK:2).
 *
 *      Returns either ERROR_BAD_FORMAT or NO_ERROR.
 *
 *      (C) Knut Stange Osmundsen. Used with permission.
 *
 *      Note that we call special (slow) memcpy versions
 *      here because the standard memcpy will fail on
 *      certain bit combinations here for some unknown
 *      reason.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC APIRET ExpandIterdata2(char *pachPage,              // out: page data (pagesize as in lx spec)
                              int cchPage,                 // in: sizeof *pachPage (pagesize as in lx spec)
                              PCSZ pachSrcPage,     // in: compressed source data in EXEPACK:1 format
                              int cchSrcPage)              // in: size of source buf
{
    char    *pachDestPage = pachPage; /* Store the pointer for boundrary checking. */

    while (cchSrcPage > 0)
    {
        /*
         * Bit 0 and 1 is the encoding type.
         */

        char cSrc = *pachSrcPage;

        switch (cSrc & 0x03)
        {
            /*
             *
             *  0  1  2  3  4  5  6  7
             *  type  |              |
             *        ----------------
             *             cch        <cch bytes of data>
             *
             * Bits 2-7 is, if not zero, the length of an uncompressed run
             *   starting at the following byte.
             *
             *  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
             *  type  |              |  |                    | |                     |
             *        ----------------  ---------------------- -----------------------
             *             zero                 cch                char to multiply
             *
             * If the bits are zero, the following two bytes describes a
             *   1 byte interation run. First byte is count, second is the byte to copy.
             *   A count of zero is means end of data, and we simply stops. In that case
             *   the rest of the data should be zero.
             */

            case 0:
            {
                if (cSrc)
                {
                    int cch = cSrc >> 2;
                    if (    (cchPage >= cch)
                         && (cchSrcPage >= cch + 1)
                       )
                    {
                        memcpy(pachPage, pachSrcPage + 1, cch);
                        pachPage += cch, cchPage -= cch;
                        pachSrcPage += cch + 1, cchSrcPage -= cch + 1;
                        break; // switch (cSrc & 0x03)
                    }
                    return ERROR_BAD_FORMAT;
                }

                if (cchSrcPage >= 2)
                {
                    int cch;
                    if (cch = pachSrcPage[1])
                    {
                        if (    (cchSrcPage >= 3)
                             && (cchPage >= cch)
                           )
                        {
                            memset(pachPage, pachSrcPage[2], cch);
                            pachPage += cch, cchPage -= cch;
                            pachSrcPage += 3, cchSrcPage -= 3;
                            break; // switch (cSrc & 0x03)
                        }
                    }
                    else
                        goto endloop;
                }

                return ERROR_BAD_FORMAT;
            }

            /*
             *  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
             *  type  |  |  |     |  |                       |
             *        ----  -------  -------------------------
             *        cch1  cch2 - 3          offset            <cch1 bytes of data>
             *
             *  Two bytes layed out as described above, followed by cch1 bytes of data to be copied.
             *  The cch2(+3) and offset describes an amount of data to be copied from the expanded
             *    data relative to the current position. The data copied as you would expect it to be.
             */

            case 1:
            {
                if (cchSrcPage >= 2)
                {
                    int off = *(PUSHORT)pachSrcPage >> 7;
                    int cch1 = cSrc >> 2 & 3;
                    int cch2 = (cSrc >> 4 & 7) + 3;
                    pachSrcPage += 2, cchSrcPage -= 2;
                    if (    (cchSrcPage >= cch1)
                         && (cchPage >= cch1 + cch2)
                         && (pachPage + cch1 - off >= pachDestPage)
                       )
                    {
                        memcpy(pachPage, pachSrcPage, cch1);
                        pachPage += cch1, cchPage -= cch1;
                        pachSrcPage += cch1, cchSrcPage -= cch1;
                        memcpyb(pachPage, pachPage - off, cch2); //memmove doesn't do a good job here for some stupid reason.
                        pachPage += cch2, cchPage -= cch2;
                        break; // switch (cSrc & 0x03)
                    }
                }
                return ERROR_BAD_FORMAT;
            }

            /*
             *  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
             *  type  |  |  |                                |
             *        ----  ----------------------------------
             *       cch-3              offset
             *
             *  Two bytes layed out as described above.
             *  The cch(+3) and offset describes an amount of data to be copied from the expanded
             *  data relative to the current position.
             *
             *  If offset == 1 the data is not copied as expected, but in the memcpyw manner.
             */

            case 2:
            {
                if (cchSrcPage >= 2)
                {
                    int off = *(PUSHORT)pachSrcPage >> 4;
                    int cch = (cSrc >> 2 & 3) + 3;
                    pachSrcPage += 2, cchSrcPage -= 2;
                    if (    (cchPage >= cch)
                         && (pachPage - off >= pachDestPage)
                       )
                    {
                        memcpyw(pachPage, pachPage - off, cch);
                        pachPage += cch, cchPage -= cch;
                        break; // switch (cSrc & 0x03)
                    }
                }
                return ERROR_BAD_FORMAT;
            }

            /*
             *  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
             *  type  |        |  |              |  |                                |
             *        ----------  ----------------  ----------------------------------
             *           cch1           cch2                     offset                <cch1 bytes of data>
             *
             *  Three bytes layed out as described above, followed by cch1 bytes of data to be copied.
             *  The cch2 and offset describes an amount of data to be copied from the expanded
             *  data relative to the current position.
             *
             *  If offset == 1 the data is not copied as expected, but in the memcpyw manner.
             */

            case 3:
            {
                if (cchSrcPage >= 3)
                {
                    int cch1 = cSrc >> 2 & 0x000f;
                    int cch2 = *(PUSHORT)pachSrcPage >> 6 & 0x003f;
                    int off  = *(PUSHORT)(pachSrcPage + 1) >> 4;
                    pachSrcPage += 3, cchSrcPage -= 3;
                    if (    (cchSrcPage >= cch1)
                         && (cchPage >= cch1 + cch2)
                         && (pachPage - off + cch1 >= pachDestPage)
                       )
                    {
                        memcpy(pachPage, pachSrcPage, cch1);
                        pachPage += cch1, cchPage -= cch1;
                        pachSrcPage += cch1, cchSrcPage -= cch1;
                        memcpyw(pachPage, pachPage - off, cch2);
                        pachPage += cch2, cchPage -= cch2;
                        break; // switch (cSrc & 0x03)
                    }
                }
                return ERROR_BAD_FORMAT;
            }
        } // end switch (cSrc & 0x03)
    }

endloop:;

    /*
     * Zero the rest of the page.
     */
    if (cchPage > 0)
        memset(pachPage, 0, cchPage);

    return 0;
}

/*
 *@@ GetOfsFromPageTableIndex:
 *      returns the offset from the executable start
 *      for the given page table index. This checks
 *      the given index and returns ERROR_INVALID_SEGMENT_NUMBER
 *      if it is invalid (for safety).
 *
 *      Otherwise this returns NO_ERROR and sets
 *      *pulPageOfs, *pulFlags, and *pulSize to
 *      the respective fields from the page table
 *      entry automatically.
 *
 *      Note that the caller must manually add the
 *      page data offset for resources (or whatever
 *      other offset from the LX header is applicable).
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

STATIC APIRET GetOfsFromPageTableIndex(PEXECUTABLE pExec,   // in: executable from exehOpen
                                       ULONG ulObjPageTblIndexThis,  // in: object page table index to look for
                                       PULONG pulFlags,        // out: page flags
                                       PULONG pulSize,         // out: page size
                                       PULONG pulPageOfs)      // out: page ofs (add pLXHeader->ulDataPagesOfs to this)
{
    OBJECTPAGETABLEENTRY *pObjPageTblEntry;

    // watch out for out of range, or we'll trap
    if (ulObjPageTblIndexThis - 1 >= pExec->pLXHeader->ulPageCount)
    {
        // _Pmpf(("ulObjPageTblIndexThis %d is too large", ulObjPageTblIndexThis));
        return ERROR_INVALID_SEGMENT_NUMBER;     // 180
    }

    pObjPageTblEntry = &pExec->pObjPageTbl[ulObjPageTblIndexThis - 1];

    // page offset: shift left by what was specified in LX header
    *pulPageOfs     =    pObjPageTblEntry->o32_pagedataoffset
                      << pExec->pLXHeader->ulPageLeftShift;
    *pulFlags       = pObjPageTblEntry->o32_pageflags;
    *pulSize        = pObjPageTblEntry->o32_pagesize;

    return NO_ERROR;
}

/*
 *@@ exehReadLXPage:
 *      loads and possibly unpacks one LX page.
 *
 *      In order to reduce memory allocations, the
 *      caller is responsible for allocating a temp
 *      buffer, which must be passed in with
 *      pabCompressed.
 *
 *      Returns:
 *
 *      --  NO_ERROR: pbData was filled with data,
 *          which is pLXHeader->ulPageSize in size.
 *
 *      --  ERROR_INVALID_SEGMENT_NUMBER: segment
 *          number is out of range.
 *
 *      --  ERROR_BAD_FORMAT: compressed page data
 *          is screwed somehow, or page size is
 *          too large.
 *
 *      plus the error codes of doshReadAt.
 *
 *@@added V0.9.16 (2002-01-05) [umoeller]
 */

APIRET exehReadLXPage(PEXECUTABLE pExec,        // in: executable from exehOpen
                      ULONG ulObjPageTblIndex,  // in: page table index to read
                      ULONG ulExeOffset,        // in: for resources, pLXHeader->ulDataPagesOfs
                      PBYTE pabCompressed,      // in: ptr to temp buffer which must be
                                                // pLXHeader->ulPageSize + 4 bytes in size
                      PBYTE pbData)             // out: ptr to buffer which receives actual
                                                // uncompressed page data (pLXHeader->ulPageSize)
{
    APIRET  arc;
    ULONG   ulFlags,
            ulSize,
            ulOffset;

    if (!(arc = GetOfsFromPageTableIndex(pExec,
                                         ulObjPageTblIndex,
                                         &ulFlags,
                                         &ulSize,
                                         &ulOffset)))
    {
        ULONG ulPageSize = pExec->pLXHeader->ulPageSize;

        // if the data is not compressed, we read it directly
        // into caller's pbData buffer (avoid one memcpy)
        // V1.0.2 (2003-11-13) [umoeller]
        PBYTE pbTarget =
            (ulFlags == VALID) ? pbData : pabCompressed;

        ulOffset += ulExeOffset;

        /* _Pmpf(("  reading pgtbl %d, ofs %d, type %s",
                ulObjPageTblIndex,
                ulOffset,
                (ulFlags == 0x0001) ? "ITERDATA"
                : (ulFlags == 0x0005) ? "ITERDATA2"
                : "uncompressed")); */

        if (ulSize > ulPageSize)
            arc = ERROR_BAD_FORMAT;
        // go read the page data (might be compressed)
        else if (!(arc = doshReadAt(pExec->pFile,
                                    ulOffset,
                                    &ulSize,
                                    pbTarget, // pabCompressed, V1.0.2 (2003-11-13) [umoeller]
                                    0)))
        {
            // _Pmpf(("   %d bytes read", ulSize));

            // terminate buffer for decompress
            *(PULONG)(pabCompressed + ulSize) = 0;

            switch (ulFlags)
            {
                case ITERDATA:
                    // OS/2 2.x:
                    arc = ExpandIterdata1(pbData,
                                          ulPageSize,
                                          pabCompressed,
                                          ulSize);            // this page's size
                break;

                case ITERDATA2:
                    // Warp 3:
                    arc = ExpandIterdata2(pbData,
                                          ulPageSize,
                                          pabCompressed,
                                          ulSize);            // this page's size
                break;

                /* V1.0.2 (2003-11-13) [umoeller]
                case VALID:
                    // uncompressed
                    memcpy(pbData,
                           pabCompressed,
                           ulPageSize);
                break;
                */
            }
        }
    }

    return arc;
}

/*
 *@@ exehLoadLXResource:
 *      attempts to load the data of the resource
 *      with the specified type and id from an LX
 *      executable.
 *
 *      If (idResource == 0), the first resource of
 *      the specified type is loaded. Otherwise we
 *      try to find the resource of the specified
 *      type _and_ ID.
 *
 *      If NO_ERROR is returned,
 *
 *      --  *ppbResData receives a new buffer with
 *          the raw resource data, which the caller
 *          must free();
 *
 *      --  *pulOffset receives an offset into that
 *          buffer, where the actual resource data
 *          starts;
 *
 *      --  *pcbResData receives the size of the
 *          following resource data (what follows
 *          after *pulOffset).
 *
 *      The reason for this slightly complicated
 *      format is to avoid another memcpy since
 *      resource data need not necessarily be on
 *      an LX page boundary.
 *
 *      This code will properly unpack compressed
 *      pages in the executable so the returned
 *      data is always unpacked and can be used
 *      directly.
 *
 *      Otherwise this returns:
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: pExec is not
 *          LX format.
 *
 *      --  ERROR_NO_DATA: resource not found.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      plus the error codes from exehReadLXPage.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 *@@changed V0.9.16 (2002-01-05) [umoeller]: largely rewritten to handle non-first icons properly
 */

APIRET exehLoadLXResource(PEXECUTABLE pExec,     // in: executable from exehOpen
                          ULONG ulType,          // in: RT_* type (e.g. RT_POINTER)
                          ULONG idResource,      // in: resource ID or 0 for first
                          PBYTE *ppbResData,     // out: resource data (to be free()'d)
                          PULONG pulOffset,      // out: offset of actual data in buffer
                          PULONG pcbResData)     // out: size of resource data (ptr can be NULL)
{
    APIRET          arc = NO_ERROR;
    ULONG           cResources = 0;

    ULONG           ulNewHeaderOfs = 0; // V0.9.12 (2001-05-03) [umoeller]

    PLXHEADER       pLXHeader;

    *ppbResData = 0;

    /* _Pmpf((__FUNCTION__ " %s: ulType = %d, idResource %d",
            pExec->pFile->pszFilename,
            ulType, idResource)); */

    if (!(pLXHeader = pExec->pLXHeader))
        return ERROR_INVALID_EXE_SIGNATURE;

    if (pExec->cbDosExeHeader)
        // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
        ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

    if (!(cResources = pLXHeader->ulResTblCnt))
        // no resources at all:
        return ERROR_NO_DATA;

    if (!pExec->fLXMapsLoaded)
        arc = exehLoadLXMaps(pExec);

    if (!arc)
    {
        // alright, we're in:

        // run thru the resources
        PXFILE  pFile = pExec->pFile;
        BOOL fPtrFound = FALSE;

        ULONG i;
        for (i = 0;
             i < cResources;
             i++)
        {
            // ptr to resource table entry
            RESOURCETABLEENTRY *pRsEntry = &pExec->pRsTbl[i];

            // check resource type and ID
            if (    (pRsEntry->type == ulType)
                 && (    (idResource == 0)
                      || (idResource == pRsEntry->name)
                    )
               )
            {
                // hooray, found it: that was the easy part...
                // finding the actual resource data isn't that
                // trivial:

                // first find the object that the resource
                // resides in, but check the bounds
                if (pRsEntry->obj - 1 >= pLXHeader->ulObjCount)
                {
                    // _Pmpf(("pRsEntry->obj %d is too large", pRsEntry->obj));
                    arc = ERROR_INVALID_SEGMENT_NUMBER;     // 180
                }
                else
                {
                    // get the object table entry from the index
                    OBJECTTABLEENTRY *pObjTblEntry = &pExec->pObjTbl[pRsEntry->obj - 1];

                    // get the object page table index for the
                    // first resource entry in this object; to
                    // this index we will need to add something
                    // which depends on pRsEntry->offset
                    ULONG   ulObjPageTblIndex = pObjTblEntry->o32_pagemap;

                    ULONG   ulPageSize = pLXHeader->ulPageSize;

                    // if this resource has specified an
                    // offset into the object, this offset
                    // specifies the offset in the uncompressed
                    // data of the whole object:
                    // for example:
                    //      res 0       ofs 0          cb 9808
                    //      res 1       ofs 9808       cb 3344
                    //      res 2       ofs 13152      cb ...
                    // and so on.
                    // So what we need to do is read in the page
                    // where the resource offset points into (which
                    // might be somewhere in the middle):
                    ULONG   ulFirstPage =   pRsEntry->offset
                                          / ulPageSize;

                    // get the offset of the resource data into
                    // that first page:
                    ULONG   ulResOffsetInFirstPage = pRsEntry->offset % ulPageSize;

                    ULONG   ulLastPage =    (pRsEntry->offset + pRsEntry->cb - 1)
                                          / ulPageSize;

                    // and we need as many pages as the resource occupies:
                    ULONG   cPages = ulLastPage - ulFirstPage + 1;

                    ULONG   cbAlloc = 0;

                    // now allocate temporary buffers
                    PBYTE   pabCompressed = NULL,
                            pabUncompressed = NULL;

                    // 4096 bytes for each page that is read in
                    // plus 4 extra bytes to terminate for decompression
                    if (!(pabCompressed = (PBYTE)malloc(ulPageSize + 4)))
                        arc = ERROR_NOT_ENOUGH_MEMORY;
                    // 4096 * cPages for the data that is composed from that
                    else if (!(arc = doshAllocArray(cPages,
                                                    ulPageSize,
                                                    &pabUncompressed,
                                                    &cbAlloc)))
                    {
                        // current pointer into pabUncompressed
                        PBYTE   pbCurrent = pabUncompressed;

                        ULONG   ul,
                                ulPageThis;

                        /* _Pmpf(("  found RT_POINTER %d, size %d, resofs %d",
                                pRsEntry->name,
                                pRsEntry->cb,
                                pRsEntry->offset));
                        _Pmpf(("  ulFirstPage %d, ulResOffsetInFirstPage %d, cPages %d",
                                ulFirstPage, ulResOffsetInFirstPage, cPages)); */

                        ulPageThis = ulObjPageTblIndex + ulFirstPage;

                        // now go for each page:
                        for (ul = 0;
                             (ul < cPages) && (!arc);
                             ul++, ulPageThis++)
                        {
                            if (!(arc = exehReadLXPage(pExec,
                                                       ulPageThis,
                                                       pLXHeader->ulDataPagesOfs,
                                                       pabCompressed,
                                                       pbCurrent)))
                            {
                                // got the data:
                                // advance target buffer pointer for
                                // next page
                                pbCurrent += ulPageSize;

                                // make sure we don't write too far away
                                if (pbCurrent > pabUncompressed + cbAlloc)
                                    arc = ERROR_BAD_FORMAT;
                            }
                        } // end for

                        // ok, now we got all the pages that do contain
                        // data for this resource:

                        if (!arc)
                        {
                            // new code without malloc/memcpy V1.0.2 (2003-11-13) [umoeller]
                            *ppbResData = pabUncompressed;
                            *pulOffset = ulResOffsetInFirstPage;


                            /*
                                // allocate a new buffer for caller
                                if (!(*ppbResData = (PBYTE)malloc(pRsEntry->cb)))
                                    arc = ERROR_NOT_ENOUGH_MEMORY;
                                else
                                {
                                    // copy into that buffer from the offset
                                    // into the first page and the data from
                                    // the subsequent pages too
                                    memcpy(*ppbResData,
                                           pabUncompressed + ulResOffsetInFirstPage,
                                           pRsEntry->cb);
                                }
                            */

                            if (pcbResData)
                                *pcbResData = pRsEntry->cb;

                            fPtrFound = TRUE;
                        }
                        else
                            FREE(pabUncompressed);

                        FREE(pabCompressed);
                    }
                }
            }

            if (fPtrFound || arc)
                break;

        } // end for

        if ((!fPtrFound) && (!arc))
            arc = ERROR_NO_DATA;
    }

    // _Pmpf((__FUNCTION__ ": returning %d", arc));

    return arc;
}

/*
 *@@ exehLoadOS2NEMaps:
 *      loads the the two main OS/2 NE maps into the
 *      given EXECUTABLE structure.
 *
 *      This loads:
 *
 *      1)  the OS/2 NE resource table;
 *
 *      2)  the OS/2 NE segment table.
 *
 *      Note that this is not automatically called
 *      by exehOpen to save time, since the NE
 *      maps are not needed for all the other exe
 *      functions. However, this does get called
 *      from exehLoadOS2NEResource if needed.
 *
 *      This returns:
 *
 *      --  NO_ERROR: both maps were loaded,
 *          and pExec->fOS2NEMapsLoaded was set to TRUE.
 *
 *      --  ERROR_INVALID_PARAMETER
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: pExec does
 *          not specify an NE executable, or the OS
 *          flag is != OS/2. This func does not work
 *          for Win16 executables.
 *
 *      --  ERROR_NO_DATA: at least one of the structs
 *          does not exist.
 *
 *      --  ERROR_NOT_ENOUGH_MEMORY
 *
 *      plus the error codes of doshReadAt.
 *
 *      Call exehFreeNEMaps to clean up explicitly, but
 *      that func automatically gets called by exehClose.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

APIRET exehLoadOS2NEMaps(PEXECUTABLE pExec)
{
    APIRET arc;

    PNEHEADER pNEHeader;

    if (!pExec)
        arc = ERROR_INVALID_PARAMETER;
    else if (pExec->fOS2NEMapsLoaded)
        // already loaded:
        arc = NO_ERROR;
    else if (    (pExec->ulExeFormat != EXEFORMAT_NE)
              || (pExec->ulOS != EXEOS_OS2)
              || (!(pNEHeader = pExec->pNEHeader))
            )
        arc = ERROR_INVALID_EXE_SIGNATURE;
    else
    {
        PXFILE pFile = pExec->pFile;
        ULONG ulNewHeaderOfs = 0;
        ULONG cb;

        if (pExec->cbDosExeHeader)
            // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
            ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

        // resource table
        if (    (!(arc = doshAllocArray(pNEHeader->usResSegmCount,
                                        sizeof(OS2NERESTBLENTRY),
                                        (PBYTE*)&pExec->paOS2NEResTblEntry,
                                        &cb)))
             && (!(arc = doshReadAt(pFile,
                                    pNEHeader->usResTblOfs
                                      + ulNewHeaderOfs,
                                    &cb,
                                    (PBYTE)pExec->paOS2NEResTblEntry,
                                    DRFL_FAILIFLESS)))
            )
        {
            // resource segments
            if (    (!(arc = doshAllocArray(pNEHeader->usResSegmCount,
                                            sizeof(OS2NESEGMENT),
                                            (PBYTE*)&pExec->paOS2NESegments,
                                            &cb)))
                 && (!(arc = doshReadAt(pFile,
                                        pNEHeader->usResTblOfs
                                          + ulNewHeaderOfs
                                          - cb,
                                        &cb,
                                        (PBYTE)pExec->paOS2NESegments,
                                        DRFL_FAILIFLESS)))
                )
            {
            }
        }

        if (!arc)
            pExec->fOS2NEMapsLoaded = TRUE;
        else
            exehFreeNEMaps(pExec);
    }

    return arc;
}

/*
 *@@ exehFreeNEMaps:
 *      frees data allocated by exehLoadOS2NEMaps.
 *      Gets called automatically by exehClose.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

VOID exehFreeNEMaps(PEXECUTABLE pExec)
{
    FREE(pExec->paOS2NEResTblEntry);
    FREE(pExec->paOS2NESegments);
    pExec->fOS2NEMapsLoaded = FALSE;
}

/*
 *@@ exehLoadOS2NEResource:
 *      attempts to load the data of the resource
 *      with the specified type and id from an OS/2
 *      NE executable.
 *
 *      Note that NE executables with resources in
 *      OS/2 format are very, very rare. The
 *      only OS/2 NE executables with resources I
 *      could find at this point were in an old 1.3
 *      Toolkit, but with them, this code works.
 *
 *      If (idResource == 0), the first resource of
 *      the specified type is loaded. Otherwise we
 *      try to find the resource of the specified
 *      type _and_ ID.
 *
 *      If NO_ERROR is returned, *ppbResData receives
 *      a new buffer with the raw resource data, and
 *      *pcbResData receives the size of that buffer.
 *      The caller must then free() that buffer.
 *
 *      Since NE doesn't support packing, the data is
 *      unpacked always and can be used directly.
 *
 *      Otherwise this returns:
 *
 *      --  ERROR_INVALID_EXE_SIGNATURE: pExec is not
 *          NE or not OS/2. This func does not work
 *          for Win16 executables.
 *
 *      --  ERROR_NO_DATA: resource not found.
 *
 *      --  ERROR_BAD_FORMAT: cannot handle resource format.
 *
 *@@added V0.9.16 (2001-12-08) [umoeller]
 */

APIRET exehLoadOS2NEResource(PEXECUTABLE pExec,     // in: executable from exehOpen
                             ULONG ulType,          // in: RT_* type (e.g. RT_POINTER)
                             ULONG idResource,      // in: resource ID or 0 for first
                             PBYTE *ppbResData,     // out: resource data (to be free()'d)
                             PULONG pcbResData)     // out: size of resource  data (ptr can be NULL)
{
    APIRET          arc = NO_ERROR;
    ULONG           cResources = 0;

    ULONG           ulNewHeaderOfs = 0; // V0.9.12 (2001-05-03) [umoeller]

    PNEHEADER       pNEHeader;

    if (!(pNEHeader = pExec->pNEHeader))
        return ERROR_INVALID_EXE_SIGNATURE;

    if (pExec->cbDosExeHeader)
        // executable has DOS stub: V0.9.12 (2001-05-03) [umoeller]
        ulNewHeaderOfs = pExec->DosExeHeader.ulNewHeaderOfs;

    // _Pmpf((__FUNCTION__ ": entering, checking %d resources", pNEHeader->usResSegmCount));

    if (!(cResources = pNEHeader->usResSegmCount))
        // no resources at all:
        return ERROR_NO_DATA;

    if (!pExec->fOS2NEMapsLoaded)
        arc = exehLoadOS2NEMaps(pExec);

    if (!arc)
    {
        // alright, we're in:
        PXFILE  pFile = pExec->pFile;

        // run thru the resources
        BOOL fPtrFound = FALSE;

        ULONG i;
        POS2NERESTBLENTRY pResTblEntryThis = pExec->paOS2NEResTblEntry;
        POS2NESEGMENT pSegThis = pExec->paOS2NESegments;
        for (i = 0;
             i < cResources;
             i++, pResTblEntryThis++, pSegThis++)
        {
            // check resource type and ID
            if (    (pResTblEntryThis->usType == ulType)
                 && (    (idResource == 0)
                      || (idResource == pResTblEntryThis->usID)
                    )
               )
            {
                // hooray, we found the resource...

                // look up the corresponding segment

                ULONG ulOffset = (    (ULONG)pSegThis->ns_sector
                                   << pNEHeader->usLogicalSectShift
                                 );

                ULONG cb = pSegThis->ns_cbseg;        // resource size
                PBYTE pb;
                if (!(*ppbResData = (PBYTE)malloc(cb)))
                    arc = ERROR_NOT_ENOUGH_MEMORY;
                else
                {
                    if (!(arc = doshReadAt(pFile,
                                           ulOffset,
                                           &cb,
                                           *ppbResData,
                                           DRFL_FAILIFLESS)))
                    {
                        if (pcbResData)
                            *pcbResData = cb;
                        fPtrFound = TRUE;
                    }
                    else
                        // error reading:
                        free(*ppbResData);
                }
            }

            if (fPtrFound || arc)
                break;

        } // end for

        if ((!fPtrFound) && (!arc))
            arc = ERROR_NO_DATA;
    }
    // else
        // _Pmpf(("exehLoadOS2NEMaps returned %d"));

    return arc;
}

/*
 *@@ exehClose:
 *      this closes an executable opened with exehOpen.
 *      Always call this function if NO_ERROR was returned by
 *      exehOpen.
 *
 *      This automaticall calls exehFreeLXMaps and
 *      exehFreeNEMaps.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.16 (2001-12-08) [umoeller]: fixed memory leaks
 *@@changed V0.9.16 (2001-12-08) [umoeller]: changed prototype to null the pExec ptr
 */

APIRET exehClose(PEXECUTABLE *ppExec)
{
    APIRET arc = NO_ERROR;
    PEXECUTABLE pExec;
    if (    (ppExec)
         && (pExec = *ppExec)
       )
    {
        char **papsz[15];
        ULONG ul;

                // (char**)&pExec->pDosExeHeader,
        papsz[0]=(char**)&pExec->pNEHeader;
        papsz[1]=(char**)&pExec->pLXHeader;
        papsz[2]=(char**)&pExec->pPEHeader;

        papsz[3]=&pExec->pszDescription;
        papsz[4]=&pExec->pszVendor;
        papsz[5]=&pExec->pszVersion;
        papsz[6]=&pExec->pszInfo;

        papsz[7]=&pExec->pszBuildDateTime;
        papsz[8]=&pExec->pszBuildMachine;
        papsz[9]=&pExec->pszASD;
        papsz[10]=&pExec->pszLanguage;
        papsz[11]=&pExec->pszCountry;
        papsz[12]=&pExec->pszRevision;
        papsz[13]=&pExec->pszUnknown;
        papsz[15]=&pExec->pszFixpak;

        exehFreeLXMaps(pExec);
        exehFreeNEMaps(pExec);

        // fixed the memory leaks with the missing fields,
        // turned this into a loop
        for (ul = 0;
             ul < sizeof(papsz) / sizeof(papsz[0]);
             ul++)
        {
            PSZ pThis;
            if (pThis = *papsz[ul])
            {
                free(pThis);
                *papsz[ul] = NULL;
            }
        }

        doshClose(&pExec->pFile);

        free(pExec);
        *ppExec = NULL;
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}


