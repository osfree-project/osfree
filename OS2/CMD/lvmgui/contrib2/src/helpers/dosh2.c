
/*
 *@@sourcefile dosh2.c:
 *      dosh.c contains more Control Program helper functions.
 *
 *      This file is new with V0.9.4 (2000-07-26) [umoeller].
 *
 *      As opposed to the functions in dosh.c, these require
 *      linking against other helpers. As a result, these have
 *      been separated from dosh.c to allow linking against
 *      dosh.obj only.
 *
 *      Function prefixes:
 *      --  dosh*   Dos (Control Program) helper functions
 *
 *      This has the same header as dosh.c, dosh.h.
 *
 *      The partition functions in this file are based on
 *      code which has kindly been provided by Dmitry A. Steklenev.
 *      See doshGetPartitionsList for how to use these.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\dosh.h"
 *@@added V0.9.4 (2000-07-27) [umoeller]
 */

/*
 *      This file Copyright (C) 1997-2013 Ulrich M■ller,
 *                                        Dmitry A. Steklenev.
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
#define INCL_DOSSESMGR
#define INCL_DOSQUEUES
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\ensure.h"
#include "helpers\level.h"
#include "helpers\nls.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"

#pragma hdrstop

/*
 *@@category: Helpers\Control program helpers\Miscellaneous
 */

/* ******************************************************************
 *
 *   Miscellaneous
 *
 ********************************************************************/

/*
 *@@ doshIsWarp4:
 *      checks the OS/2 system version number.
 *
 *      Returns:
 *
 *      -- 0 (FALSE): OS/2 2.x or Warp 3 is running.
 *
 *      -- 1: Warp 4.0 is running.
 *
 *      -- 2: Warp 4.5 kernel is running on Warp 4.0 (Warp 4 FP 13+).
 *
 *      -- 3: Warp 4.5 is running (WSeB or eCS or ACP/MCP), or even something newer.
 *
 *@@changed V0.9.2 (2000-03-05) [umoeller]: reported TRUE on Warp 3 also; fixed
 *@@changed V0.9.6 (2000-10-16) [umoeller]: patched for speed
 *@@changed V0.9.9 (2001-04-04) [umoeller]: now returning 2 for Warp 4.5 and above
 *@@changed V1.0.5 (2006-05-29) [pr]: now returning 3 for Warp 4.5 and above and 2 for
 *   Warp 4.0 FP13+; moved here from dosh.c
 */

ULONG doshIsWarp4(VOID)
{
    static BOOL     s_fQueried = FALSE;
    static ULONG    s_ulrc = 0;

    if (!s_fQueried)
    {
        // first call:
        ULONG       aulBuf[3];

        DosQuerySysInfo(QSV_VERSION_MAJOR,      // 11
                        QSV_VERSION_MINOR,      // 12
                        &aulBuf, sizeof(aulBuf));
        // Warp 3 is reported as 20.30
        // Warp 4 is reported as 20.40
        // Aurora is reported as 20.45 (regardless of convenience packs)

        if (aulBuf[0] > 20)        // major > 20; not the case with Warp 3, 4, 5
            s_ulrc = 3;
        else
            if (aulBuf[0] == 20)   // major == 20
                if (aulBuf[1] >= 45)   // minor >= 45  Warp 4 FP13 or later
                    s_ulrc = 2;
                else
                    if (aulBuf[1] == 40)   // minor == 40  Warp 4 pre-FP13
                        s_ulrc = 1;

        // Now check SYSLEVEL.OS2 to detect between Warp 4 and MCP
        if (s_ulrc == 2)
        {
            CHAR szName[CCHMAXPATH] = "?:\\OS2\\INSTALL\\SYSLEVEL.OS2";
            ULONG cbFile;
            PXFILE pFile;

            szName[0] = doshQueryBootDrive();
            if (!doshOpen(szName,
                          XOPEN_READ_EXISTING | XOPEN_BINARY,
                          &cbFile,
                          &pFile))
            {
                CHAR szVersion[2];
                ULONG ulSize;

                if (   !lvlQueryLevelFileData(pFile->hf,
                                              QLD_MINORVERSION,
                                              szVersion,
                                              sizeof(szVersion),
                                              &ulSize)
                    && (szVersion[0] >= '5'))   // minor >= 5  is MCP
                        s_ulrc = 3;

                doshClose(&pFile);
            }
        }

        s_fQueried = TRUE;
    }

    return (s_ulrc);
}

/*
 *@@ doshIsFixpak:
 *      checks if the OS/2 system is at a given fixpak level or higher
 *
 *      Returns:
 *
 *      -- FALSE: the system is at a lower level
 *
 *      -- TRUE: the system is at the exact or a higher level
 *
 *@@added V1.0.8 (2008-04-07) [chennecke]: @@fixes 1067
 */

BOOL doshIsFixpak(BOOL fIsMcp,          // in: MCP fixpak?
                  ULONG ulFixpakLevel)    // in: fixpak level number
{
    static BOOL     s_fQueried = FALSE;
    static BOOL     s_fRc = FALSE;

    if (!s_fQueried)
    {
        // first call:
        // check SYSLEVEL.OS2
        CHAR szName[CCHMAXPATH] = "?:\\OS2\\INSTALL\\SYSLEVEL.OS2";
        ULONG cbFile;
        PXFILE pFile;

        szName[0] = doshQueryBootDrive();
        if (!doshOpen(szName,
                      XOPEN_READ_EXISTING | XOPEN_BINARY,
                      &cbFile,
                      &pFile))
        {
            CHAR szCsdLevel[8];
            CHAR szCsdLevelNumber[4];
            ULONG ulSize;

            if (!lvlQueryLevelFileData(pFile->hf,
                                       QLD_CURRENTCSD,
                                       szCsdLevel,
                                       sizeof(szCsdLevel),
                                       &ulSize))
            {
                strncpy(szCsdLevelNumber, szCsdLevel + 4, 3);
                szCsdLevelNumber[sizeof(szCsdLevelNumber) - 1] = '\0';
                if (fIsMcp)
                {
                    if (   (szCsdLevel[3] == 'C')
                        && (atol(szCsdLevelNumber) >= ulFixpakLevel))
                        s_fRc = TRUE;
                }
                else
                {
                    if (   (szCsdLevel[3] == 'C')
                        || (    (szCsdLevel[3] == 'M')
                             && (atol(szCsdLevelNumber) >= ulFixpakLevel)))
                        s_fRc = TRUE;
                }
            }

            doshClose(&pFile);
        }

        s_fQueried = TRUE;
    }

    return (s_fRc);
}


/*
 *@@ doshIsValidFileName:
 *      this returns NO_ERROR only if pszFile is a valid file name.
 *      This may include a full path.
 *
 *      If a drive letter is specified, this checks for whether
 *      that drive is a FAT drive and adjust the checks accordingly,
 *      i.e. 8+3 syntax (per path component).
 *
 *      If no drive letter is specified, this check is performed
 *      for the current drive.
 *
 *      This also checks if pszFileNames contains characters which
 *      are invalid for the current drive.
 *
 *      Note: this performs syntactic checks only. This does not
 *      check for whether the specified path components exist.
 *      However, it _is_ checked for whether the given drive
 *      exists.
 *
 *      This func is especially useful to check filenames that
 *      have been entered by the user in a "Save as" dialog.
 *
 *      If an error is found, the corresponding DOS error code
 *      is returned:
 *      --  ERROR_INVALID_DRIVE
 *      --  ERROR_FILENAME_EXCED_RANGE  (on FAT: no 8+3 filename)
 *      --  ERROR_INVALID_NAME          (invalid character)
 *      --  ERROR_CURRENT_DIRECTORY     (if fFullyQualified: no full path specified)
 *
 *@@changed V0.9.2 (2000-03-11) [umoeller]: added fFullyQualified
 */

APIRET doshIsValidFileName(const char* pcszFile,
                           BOOL fFullyQualified)    // in: if TRUE, pcszFile must be fully q'fied
{
    APIRET  arc = NO_ERROR;
    CHAR    szPath[CCHMAXPATH+4] = " :";
    CHAR    szComponent[CCHMAXPATH];
    PSZ     p1, p2;
    BOOL    fIsFAT = FALSE;
    PSZ     pszInvalid;

    if (fFullyQualified)    // V0.9.2 (2000-03-11) [umoeller]
    {
        if (    (*(pcszFile + 1) != ':')
             || (*(pcszFile + 2) != '\\')
           )
            arc = ERROR_CURRENT_DIRECTORY;
    }

    // check drive first
    if (*(pcszFile + 1) == ':')
    {
        CHAR cDrive = toupper(*pcszFile);
        double d;
        // drive specified:
        strcpy(szPath, pcszFile);
        szPath[0] = toupper(*pcszFile);
        arc = doshQueryDiskFree(cDrive - 'A' + 1, &d);
    }
    else
    {
        // no drive specified: take current
        ULONG   ulDriveNum = 0,
                ulDriveMap = 0;
        arc = DosQueryCurrentDisk(&ulDriveNum, &ulDriveMap);
        szPath[0] = ((UCHAR)ulDriveNum) + 'A' - 1;
        szPath[1] = ':';
        strcpy(&szPath[2], pcszFile);
    }

    if (arc == NO_ERROR)
    {
        fIsFAT = doshIsFileOnFAT(szPath);

        pszInvalid = (fIsFAT)
                        ? "<>|+=:;,\"/[] "  // invalid characters in FAT
                        : "<>|:\"/";        // invalid characters in IFS's

        // now separate path components
        p1 = &szPath[2];       // advance past ':'

        do {

            if (*p1 == '\\')
                p1++;

            p2 = strchr(p1, '\\');
            if (p2 == NULL)
                p2 = p1 + strlen(p1);

            if (p1 != p2)
            {
                LONG    lDotOfs = -1,
                        lAfterDot = -1;
                ULONG   cbFile,
                        ul;
                PSZ     pSource = szComponent;

                strncpy(szComponent, p1, p2-p1);
                szComponent[p2-p1] = 0;
                cbFile = strlen(szComponent);

                // now check each path component
                for (ul = 0; ul < cbFile; ul++)
                {
                    if (fIsFAT)
                    {
                        // on FAT: only 8 characters allowed before dot
                        if (*pSource == '.')
                        {
                            lDotOfs = ul;
                            lAfterDot = 0;
                            if (ul > 7)
                                return ERROR_FILENAME_EXCED_RANGE;
                        }
                    }
                    // and check for invalid characters
                    if (strchr(pszInvalid, *pSource) != NULL)
                        return ERROR_INVALID_NAME;

                    pSource++;

                    // on FAT, allow only three chars after dot
                    if (fIsFAT)
                        if (lAfterDot != -1)
                        {
                            lAfterDot++;
                            if (lAfterDot > 3)
                                return ERROR_FILENAME_EXCED_RANGE;
                        }
                }

                // we are still missing the case of a FAT file
                // name without extension; if so, check whether
                // the file stem is <= 8 chars
                if (fIsFAT)
                    if (lDotOfs == -1)  // dot not found:
                        if (cbFile > 8)
                            return ERROR_FILENAME_EXCED_RANGE;
            }

            // go for next component
            p1 = p2+1;
        } while (*p2);
    }

    return arc;
}

/*
 *@@ doshMakeRealName:
 *      this copies pszSource to pszTarget, replacing
 *      all characters which are not supported by file
 *      systems with cReplace.
 *
 *      pszTarget must be at least the same size as pszSource.
 *      If (fIsFAT), the file name will be made FAT-compliant (8+3).
 *
 *      Returns TRUE if characters were replaced.
 *
 *@@changed V0.9.0 (99-11-06) [umoeller]: now replacing "*" too
 */

BOOL doshMakeRealName(PSZ pszTarget,    // out: new real name
                      PSZ pszSource,    // in: filename to translate
                      CHAR cReplace,    // in: replacement char for invalid
                                        //     characters (e.g. '!')
                      BOOL fIsFAT)      // in: make-FAT-compatible flag
{
    ULONG ul,
          cbSource = strlen(pszSource);
    LONG  lDotOfs = -1,
          lAfterDot = -1;
    BOOL  brc = FALSE;
    PSZ   pSource = pszSource,
          pTarget = pszTarget;

    const char *pcszInvalid = (fIsFAT)
                                   ? "*<>|+=:;,\"/\\[] "  // invalid characters in FAT
                                   : "*<>|:\"/\\"; // invalid characters in IFS's

    for (ul = 0; ul < cbSource; ul++)
    {
        if (fIsFAT)
        {
            // on FAT: truncate filename if neccessary
            if (*pSource == '.')
            {
                lDotOfs = ul;
                lAfterDot = 0;
                if (ul > 7) {
                    // only 8 characters allowed before dot,
                    // so set target ptr to dot pos
                    pTarget = pszTarget+8;
                }
            }
        }
        // and replace invalid characters
        if (strchr(pcszInvalid, *pSource) == NULL)
            *pTarget = *pSource;
        else
        {
            *pTarget = cReplace;
            brc = TRUE;
        }
        pTarget++;
        pSource++;

        // on FAT, allow only three chars after dot
        if (fIsFAT)
            if (lAfterDot != -1)
            {
                lAfterDot++;
                if (lAfterDot > 3)
                    break;
            }
    }
    *pTarget = '\0';

    if (fIsFAT)
    {
        // we are still missing the case of a FAT file
        // name without extension; if so, check whether
        // the file stem is <= 8 chars
        if (lDotOfs == -1)  // dot not found:
            if (cbSource > 8)
                *(pszTarget+8) = 0; // truncate

        // convert to upper case
        strupr(pszTarget);
    }

    return brc;
}

/*
 *@@ doshSetCurrentDir:
 *      sets the current working directory
 *      to the given path.
 *
 *      As opposed to DosSetCurrentDir, this
 *      one will change the current drive
 *      also, if one is specified.
 *
 *@@changed V0.9.9 (2001-04-04) [umoeller]: this returned an error even if none occurred, fixed
 */

APIRET doshSetCurrentDir(const char *pcszDir)
{
    APIRET  arc = NO_ERROR;

    if (    (!pcszDir)
         || (!(*pcszDir))
       )
        return ERROR_INVALID_PARAMETER;

    if (pcszDir[1] == ':')
    {
        // drive given:
        CHAR    cDrive = toupper(*(pcszDir));
        // change drive
        arc = DosSetDefaultDisk( (ULONG)(cDrive - 'A' + 1) );
                // 1 = A:, 2 = B:, ...
    }

    if (!arc)
        arc = DosSetCurrentDir((PSZ)pcszDir);

    return arc;       // V0.9.9 (2001-04-04) [umoeller]
}

/*
 *@@ CopyToBuffer:
 *      little helper for copying a string to
 *      a target buffer with length checking.
 *
 *      Returns:
 *
 *      --  NO_ERROR
 *
 *      --  ERROR_BUFFER_OVERFLOW if pszTarget does
 *          not have enough room to hold pcszSource
 *          (including the null terminator).
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 */

STATIC APIRET CopyToBuffer(PSZ pszTarget,      // out: target buffer
                           PCSZ pcszSource,    // in: source string
                           ULONG cbTarget)     // in: size of target buffer
{
    ULONG ulLength = strlen(pcszSource);
    if (ulLength < cbTarget)
    {
        memcpy(pszTarget,
               pcszSource,
               ulLength + 1);
        return NO_ERROR;
    }

    return ERROR_BUFFER_OVERFLOW;
}

/*
 *@@ doshSearchPath:
 *      replacement for DosSearchPath.
 *
 *      This looks along all directories which are
 *      specified in the value of the given environment
 *      variable if pcszFile is found.
 *
 *      As opposed to the stupid DosSearchPath, this
 *      ignores subdirectories in the path particles.
 *      For example, DosSearchPath would usually not
 *      find an INSTALL file because \OS2 contains
 *      an INSTALL directory, or NETSCAPE because
 *      \OS2\INSTALL contains a NETSCAPE directory.
 *
 *      Returns:
 *
 *      --  NO_ERROR: pszExecutable has received the
 *          full path of pcszFile.
 *
 *      --  ERROR_FILE_NOT_FOUND: pcszFile was not found
 *          in the specified path (or is a directory).
 *
 *      --  ERROR_BUFFER_OVERFLOW: pcszFile was found, but
 *          the pszExecutable buffer is too small to hold
 *          the full path.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 *@@changed V1.0.0 (2002-11-23) [umoeller]: allowing NULL pcszPath to search for "PATH"
 */

APIRET doshSearchPath(const char *pcszPath,     // in: path variable name; if NULL, we use "PATH"
                      const char *pcszFile,     // in: file to look for (e.g. "LVM.EXE")
                      PSZ pszExecutable,        // out: full path (e.g. "F:\os2\lvm.exe")
                      ULONG cbExecutable)       // in: sizeof (*pszExecutable)
{
    APIRET arc = NO_ERROR;

    // get the PATH value
    PCSZ pcszPathValue;

    if (!pcszPath)
        pcszPath = "PATH";      // V1.0.0 (2002-11-23) [umoeller]

    if (!(arc = DosScanEnv((PSZ)pcszPath,
#if __cplusplus
                           &pcszPathValue)))
#else
                           (PSZ*)&pcszPathValue)))
#endif
    {
        // run thru the path components
        PSZ pszPathCopy;
        if (pszPathCopy = strdup(pcszPathValue))
        {
            PSZ pszToken = strtok(pszPathCopy, ";");
            while (pszToken)
            {
                CHAR szFileMask[2*CCHMAXPATH];
                FILESTATUS3 fs3;

                sprintf(szFileMask,
                        "%s\\%s",
                        pszToken,           // path particle
                        pcszFile);          // e.g. "netscape"

                if (    (!(arc = DosQueryPathInfo(szFileMask,
                                                  FIL_STANDARD,
                                                  &fs3,
                                                  sizeof(fs3))))
                     // make sure it's not a directory
                     // and that it's not hidden
                     && (!(fs3.attrFile & (FILE_DIRECTORY | FILE_HIDDEN)))
                   )
                {
                    // copy
                    arc = CopyToBuffer(pszExecutable,
                                       szFileMask,
                                       cbExecutable);
                    // and stop
                    break;
                }
                else
                    arc = ERROR_FILE_NOT_FOUND;
                    // and search on

                pszToken = strtok(NULL, ";");
            };

            free(pszPathCopy);
        }
        else
            arc = ERROR_NOT_ENOUGH_MEMORY;
    }

    return arc;
}

/*
 *@@ doshSearchDirs:
 *      This looks along all directories which are
 *      specified in the passed directory list
 *      if pcszFile is found.
 *
 *      As opposed to the stupid DosSearchPath, this
 *      ignores subdirectories in the path particles.
 *      For example, DosSearchPath would usually not
 *      find an INSTALL file because \OS2 contains
 *      an INSTALL directory, or NETSCAPE because
 *      \OS2\INSTALL contains a NETSCAPE directory.
 *
 *      Returns:
 *
 *      --  NO_ERROR: pszExecutable has received the
 *          full path of pcszFile.
 *
 *      --  ERROR_FILE_NOT_FOUND: pcszFile was not found
 *          in the specified path (or is a directory).
 *
 *      --  ERROR_BUFFER_OVERFLOW: pcszFile was found, but
 *          the pszExecutable buffer is too small to hold
 *          the full path.
 *
 *@@added V1.0.4 (2005-06-16) [chennecke]: blatantly stolen from doshSearchPath
 */

APIRET doshSearchDirs(const char *pcszDirList,  // in: list of directories in PATH style
                      const char *pcszFile,     // in: file to look for (e.g. "LVM.EXE")
                      PSZ pszExecutable,        // out: full path (e.g. "F:\os2\lvm.exe")
                      ULONG cbExecutable)       // in: sizeof (*pszExecutable)
{
    APIRET arc = NO_ERROR;

    // run thru the path components
    PSZ pszPathCopy;
    if (pszPathCopy = strdup(pcszDirList))
    {
        PSZ pszToken = strtok(pszPathCopy, ";");
        while (pszToken)
        {
            CHAR szFileMask[2*CCHMAXPATH];
            FILESTATUS3 fs3;

            sprintf(szFileMask,
                    "%s\\%s",
                    pszToken,           // path particle
                    pcszFile);          // e.g. "netscape"

            if (    (!(arc = DosQueryPathInfo(szFileMask,
                                              FIL_STANDARD,
                                              &fs3,
                                              sizeof(fs3))))
                 // make sure it's not a directory
                 // and that it's not hidden
                 && (!(fs3.attrFile & (FILE_DIRECTORY | FILE_HIDDEN)))
               )
            {
                // copy
                arc = CopyToBuffer(pszExecutable,
                                   szFileMask,
                                   cbExecutable);
                // and stop
                break;
            }
            else
                arc = ERROR_FILE_NOT_FOUND;
                // and search on

            pszToken = strtok(NULL, ";");
        };

        free(pszPathCopy);
    }
    else
        arc = ERROR_NOT_ENOUGH_MEMORY;

    return arc;
}

/*
 * FindFile:
 *      helper for doshFindExecutable.
 *
 *added V0.9.11 (2001-04-25) [umoeller]
 *@@changed V0.9.16 (2001-10-08) [umoeller]: rewrote second half for DosSearchPath replacement, which returns directories too
 */

STATIC APIRET FindFile(const char *pcszCommand,      // in: command (e.g. "lvm")
                       PSZ pszExecutable,            // out: full path (e.g. "F:\os2\lvm.exe")
                       ULONG cbExecutable)           // in: sizeof (*pszExecutable)
{
    APIRET arc = NO_ERROR;
    FILESTATUS3 fs3;

    if (    (strchr(pcszCommand, '\\'))
         || (strchr(pcszCommand, ':'))
       )
    {
        // looks like this is qualified:
        arc = DosQueryPathInfo((PSZ)pcszCommand,
                               FIL_STANDARD,
                               &fs3,
                               sizeof(fs3));
        if (!arc)
            if (!(fs3.attrFile & FILE_DIRECTORY))
                arc = CopyToBuffer(pszExecutable,
                                   pcszCommand,
                                   cbExecutable);
            else
                // directory:
                arc = ERROR_INVALID_EXE_SIGNATURE;
    }
    else
    {
        // non-qualified:
        /* arc = DosSearchPath(SEARCH_IGNORENETERRS
                                | SEARCH_ENVIRONMENT
                                | SEARCH_CUR_DIRECTORY,
                            "PATH",
                            (PSZ)pcszCommand,
                            pszExecutable,
                            cbExecutable); */
            // The above is not useable. It returns directories
            // on the path... for example, it returns \OS2\INSTALL\NETSCAPE
            // if netscape is looked for. So we search manually... sigh.
            // V0.9.16 (2001-10-08) [umoeller]
        arc = doshSearchPath("PATH",
                             pcszCommand,
                             pszExecutable,
                             cbExecutable);
    }

    return arc;
}

/*
 *@@ doshFindExecutable:
 *      this attempts to find an executable by doing the
 *      following:
 *
 *      1)  If pcszCommand appears to be qualified (i.e. contains
 *          a backslash), this checks for whether the file exists.
 *          If it is a directory, ERROR_INVALID_EXE_SIGNATURE is
 *          returned.
 *
 *      2)  If pcszCommand contains no backslash, this searches
 *          all directories on the PATH in order to find the full
 *          path of the executable. Starting with V0.9.16, we
 *          use doshSearchPath for that.
 *
 *      papcszExtensions determines if additional searches are to be
 *      performed if the file doesn't exist (case 1) or doshSearchPath
 *      returned ERROR_FILE_NOT_FOUND (case 2).
 *      This must point to an array of strings specifying the extra
 *      extensions to search for.
 *
 *      If both papcszExtensions and cExtensions are null, no
 *      extra searches are performed.
 *
 *      Returns:
 *
 *      --  NO_ERROR: pszExecutable has received the full path of
 *          the executable found by DosSearchPath.
 *
 *      --  ERROR_FILE_NOT_FOUND
 *
 *      --  ERROR_BUFFER_OVERFLOW: pcszCommand was found, but
 *          the pszExecutable buffer is too small to hold
 *          the full path.
 *
 *      Example:
 *
 +      const char *aExtensions[] = {  "EXE",
 +                                     "COM",
 +                                     "CMD"
 +                                  };
 +      CHAR szExecutable[CCHMAXPATH];
 +      APIRET arc = doshFindExecutable("lvm",
 +                                      szExecutable,
 +                                      sizeof(szExecutable),
 +                                      aExtensions,
 +                                      3);
 *
 *@@added V0.9.9 (2001-03-07) [umoeller]
 *@@changed V0.9.11 (2001-04-25) [umoeller]: this never worked for qualified pcszCommand's, fixed
 */

APIRET doshFindExecutable(const char *pcszCommand,      // in: command (e.g. "lvm")
                          PSZ pszExecutable,            // out: full path (e.g. "F:\os2\lvm.exe")
                          ULONG cbExecutable,           // in: sizeof (*pszExecutable)
                          const char **papcszExtensions, // in: array of extensions (without dots)
                          ULONG cExtensions)            // in: array item count
{
    APIRET arc = FindFile(pcszCommand,
                          pszExecutable,
                          cbExecutable);

    if (    (arc == ERROR_FILE_NOT_FOUND)           // not found?
         && (cExtensions)                    // any extra searches wanted?
       )
    {
        // try additional things then
        PSZ psz2;
        if (psz2 = (PSZ)malloc(strlen(pcszCommand) + 20))
        {
            ULONG   ul;
            for (ul = 0;
                 ul < cExtensions;
                 ul++)
            {
                const char *pcszExtThis = papcszExtensions[ul];
                sprintf(psz2,
                        "%s.%s",
                        pcszCommand,
                        pcszExtThis);
                arc = FindFile(psz2,
                               pszExecutable,
                               cbExecutable);
                if (arc != ERROR_FILE_NOT_FOUND)
                    break;
            }

            free(psz2);
        }
        else
            arc = ERROR_NOT_ENOUGH_MEMORY;
    }

    return arc;
}

/*
 *@@category: Helpers\Control program helpers\Partitions info
 *      functions for retrieving partition information directly
 *      from the partition tables on the disk. See doshGetPartitionsList.
 */

/********************************************************************
 *
 *   Partition functions
 *
 ********************************************************************/

/*
 *@@ doshQueryDiskCount:
 *      returns the no. of physical disks installed
 *      on the system.
 *
 *@@added V0.9.0 [umoeller]
 */

UINT doshQueryDiskCount(VOID)
{
    USHORT usCount = 0;
    DosPhysicalDisk(INFO_COUNT_PARTITIONABLE_DISKS, &usCount, 2, 0, 0);
    return usCount;
}

/*
 *@@ doshType2FSName:
 *      this returns a static, zero-terminated string
 *      for the given FS type, or NULL if the type
 *      is unknown.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.16 (2001-10-08) [umoeller]: rewritten
 */

const char* doshType2FSName(unsigned char bFSType)  // in: FS type
{
    switch (bFSType)
    {
        case 0x00: return "empty";
        case 0x01: return "DOS 12-bit FAT < 10 Mb";
        case 0x02: return "XENIX root file system";
        case 0x03: return "XENIX /usr file system (obsolete)";
        case 0x04: return "DOS 16-bit FAT < 32 Mb";
        case 0x05: return "DOS 3.3+ extended partition";
        case 0x06: return "DOS 3.31+ 16-bit FAT > 32 Mb";
        case 0x07: return "HPFS/NTFS/QNX/Advanced Unix";
        case 0x08: return "OS/2 1.0-1.3/AIX/Commodore/DELL";
        case 0x09: return "AIX data/Coherent";
        case 0x0A: return "OS/2 Boot Manager/OPUS/Coherent Swap";
        case 0x0B: return "Windows95 with 32-bit FAT";
        case 0x0C: return "Windows95 with 32-bit FAT (LBA)";
        case 0x0E: return "Windows 95 VFAT (06h plus LBA)";
        case 0x0F: return "Windows 95 VFAT (05h plus LBA)";
        case 0x10: return "OPUS";
        case 0x11: return "OS/2 Boot Manager hidden 12-bit FAT";
        case 0x12: return "Compaq Diagnostics";
        case 0x14: return "OS/2 Boot Manager hidden sub-32M 16-bit FAT";
        case 0x16: return "OS/2 Boot Manager hidden over-32M 16-bit FAT";
        case 0x17: return "OS/2 Boot Manager hidden HPFS";
        case 0x18: return "AST special Windows swap file (\"Zero-Volt Suspend\")";
        // case 0x21: reserved
        // case 0x23: reserved
        case 0x24: return "NEC MS-DOS 3.x";
        // case 0x26: reserved
        // case 0x31: reserved
        // case 0x33: reserved
        // case 0x34: reserved
        case 0x35: return "JFS";
        // case 0x36: reserved
        case 0x38: return "Theos";
        case 0x3C: return "PowerQuest PartitionMagic recovery partition";
        case 0x40: return "VENIX 80286";
        case 0x41: return "Personal RISC Boot";
        case 0x42: return "SFS (Secure File System) by Peter Gutmann";
        case 0x50: return "OnTrack Disk Manager, read-only";
        case 0x51: return "OnTrack Disk Manager, read/write";
        case 0x52: return "CP/M or Microport System V/386";
        case 0x53: return "OnTrack Disk Manager, write-only???";
        case 0x54: return "OnTrack Disk Manager (DDO)";
        case 0x56: return "GoldenBow VFeature";
        case 0x61: return "SpeedStor";
        case 0x63: return "Unix SysV/386, 386/ix or Mach, MtXinu BSD 4.3 on Mach or GNU HURD";
        case 0x64: return "Novell NetWare 286";
        case 0x65: return "Novell NetWare (3.11)";
        case 0x67:
        case 0x68:
        case 0x69: return "Novell";
        case 0x70: return "DiskSecure Multi-Boot";
        // case 0x71: reserved
        // case 0x73: reserved
        // case 0x74: reserved
        case 0x75: return "PC/IX";
        // case 0x76: reserved
        case 0x80: return "Minix v1.1 - 1.4a";
        case 0x81: return "Minix v1.4b+ or Linux or Mitac Advanced Disk Manager";
        case 0x82: return "Linux Swap or Prime";
        case 0x83: return "Linux native file system (ext2fs/xiafs)";
        case 0x84: return "OS/2-renumbered type 04h (hidden DOS C: drive)";
        case 0x86: return "FAT16 volume/stripe set (Windows NT)";
        case 0x87: return "HPFS Fault-Tolerant mirrored partition or NTFS volume/stripe set";
        case 0x93: return "Amoeba file system";
        case 0x94: return "Amoeba bad block table";
        case 0xA0: return "Phoenix NoteBIOS Power Management \"Save-to-Disk\" partition";
        // case 0xA1: reserved
        // case 0xA3: reserved
        // case 0xA4: reserved
        case 0xA5: return "FreeBSD, BSD/386";
        // case 0xA6: reserved
        // case 0xB1: reserved
        // case 0xB3: reserved
        // case 0xB4: reserved
        // case 0xB6: reserved
        case 0xB7: return "BSDI file system (secondarily swap)";
        case 0xB8: return "BSDI swap (secondarily file system)";
        case 0xC1: return "DR DOS 6.0 LOGIN.EXE-secured 12-bit FAT";
        case 0xC4: return "DR DOS 6.0 LOGIN.EXE-secured 16-bit FAT";
        case 0xC6: return "DR DOS 6.0 LOGIN.EXE-secured Huge partition or NT corrupted FAT16 volume/stripe set";
        case 0xC7: return "Syrinx Boot or corrupted NTFS volume/stripe set";
        case 0xD8: return "CP/M-86";
        case 0xDB: return "CP/M, Concurrent CP/M, Concurrent DOS, Convergent Technologies OS";
        case 0xE1: return "SpeedStor 12-bit FAT extended partition";
        case 0xE3: return "DOS read-only or Storage Dimensions";
        case 0xE4: return "SpeedStor 16-bit FAT extended partition";
        // case 0xE5: reserved
        // case 0xE6: reserved
        case 0xF1: return "Storage Dimensions";
        case 0xF2: return "DOS 3.3+ secondary partition";
        // case 0xF3: reserved
        case 0xF4: return "SpeedStor or Storage Dimensions";
        // case 0xF6: reserved
        case 0xFE: return "LANstep or IBM PS/2 IML";
        case 0xFF: return "Xenix bad block table";
    }

    return NULL;
}

/*
 * AppendPartition:
 *      this appends the given partition information to
 *      the given partition list. To do this, a new
 *      PARTITIONINFO structure is created and appended
 *      in a list (managed thru the PARTITIONINFO.pNext
 *      items).
 *
 *      pppiThis must be a pointer to a pointer to a PARTITIONINFO.
 *      With each call of this function, this pointer is advanced
 *      to point to the newly created PARTITIONINFO, so before
 *      calling this function for the first time,
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.20 (2002-08-10) [umoeller]: fixed truncated LVM names
 */

STATIC APIRET AppendPartition(PARTITIONINFO **pppiFirst,
                              PARTITIONINFO **pppiThis,    // in/out: partition info; pointer will be advanced
                              PUSHORT posCount,            // in/out: partition count
                              BYTE bDisk,                  // in: disk of partition
                              const char *pszBootName,     // in: boot partition name
                              CHAR cLetter,                // in/out: drive letter
                              BYTE bFsType,                // in: file system type
                              BOOL fPrimary,               // in: primary?
                              BOOL fBootable,
                              ULONG ulSectors)             // in: no. of sectors
{
    APIRET arc = NO_ERROR;
    PPARTITIONINFO ppiNew;
    if (ppiNew = NEW(PARTITIONINFO))
    {
        ZERO(ppiNew);

        // store data
        ppiNew->bDisk = bDisk;
        if (    (fBootable)
             && (pszBootName)
           )
        {
            // fixed truncated LVM names V0.9.20 (2002-08-10) [umoeller]
            strhncpy0(ppiNew->szBootName,
                      pszBootName,
                      sizeof(ppiNew->szBootName));
        }
        else
            ppiNew->szBootName[0] = 0;
        ppiNew->cLetter = cLetter;
        ppiNew->bFSType = bFsType;
        ppiNew->pcszFSType = doshType2FSName(bFsType);
        ppiNew->fPrimary = fPrimary;
        ppiNew->fBootable = fBootable;
        ppiNew->ulSize = ulSectors / 2048;

        ppiNew->pNext = NULL;

        (*posCount)++;

        if (*pppiFirst == (PPARTITIONINFO)NULL)
        {
            // first call:
            *pppiFirst = ppiNew;
            *pppiThis = ppiNew;
        }
        else
        {
            // append to list
            (**pppiThis).pNext = ppiNew;
            *pppiThis = ppiNew;
        }
    }
    else
        arc = ERROR_NOT_ENOUGH_MEMORY;

    return arc;
}

#ifndef __XWPLITE__

/*
 *@@ doshReadSector:
 *      reads a physical disk sector.
 *
 *      If NO_ERROR is returned, the sector contents
 *      have been stored in *buff.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.9 (2001-04-04) [umoeller]: added more error checking
 */

APIRET doshReadSector(USHORT disk,      // in: physical disk no. (1, 2, 3, ...)
                      void *buff,
                      USHORT head,
                      USHORT cylinder,
                      USHORT sector)
{
    APIRET  arc;
    HFILE   dh = 0;
    char    dn[256];

    sprintf(dn, "%u:", disk);
    if (!(arc = DosPhysicalDisk(INFO_GETIOCTLHANDLE, &dh, 2, dn, 3)))
    {
        TRACKLAYOUT DiskIOParm;
        ULONG IOCtlDataLength = sizeof(DiskIOParm);
        ULONG IOCtlParmLength = 512;

        DiskIOParm.bCommand = 0;
        DiskIOParm.usHead = head;
        DiskIOParm.usCylinder = cylinder;
        DiskIOParm.usFirstSector = 0;
        DiskIOParm.cSectors = 1;
        DiskIOParm.TrackTable[0].usSectorNumber = sector;
        DiskIOParm.TrackTable[0].usSectorSize = 512;

        arc = DosDevIOCtl(dh,
                          IOCTL_PHYSICALDISK, PDSK_READPHYSTRACK,
                          &DiskIOParm, IOCtlParmLength, &IOCtlParmLength,
                          buff       , IOCtlDataLength, &IOCtlDataLength);

        DosPhysicalDisk(INFO_FREEIOCTLHANDLE, 0, 0, &dh, 2);
    }

    return arc;
}

// Sector and Cylinder values are actually 6 bits and 10 bits:
//
//   1 1 1 1 1 1
//  ???????????????бд?
//  Ёc c c c c c c c C c S s s s s s?
//  юдададададададададададададададад?
//
// The high two bits of the second byte are used as the high bits
// of a 10-bit value.  This allows for as many as 1024 cylinders
// and 64 sectors per cylinder.

/*
 * GetCyl:
 *      get cylinder number.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 */

STATIC USHORT GetCyl(USHORT rBeginSecCyl)
{
    return   ((rBeginSecCyl & 0x00C0) << 2)
           + ((rBeginSecCyl & 0xFF00) >> 8);
}

/*
 * GetSec:
 *      get sector number.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 */

STATIC USHORT GetSec(USHORT rBeginSecCyl)
{
    return rBeginSecCyl & 0x003F;
}

/*
 *@@ doshGetBootManager:
 *      this goes thru the master boot records on all
 *      disks to find the boot manager partition.
 *
 *      Returns:
 *
 *      -- NO_ERROR: boot manager found; in that case,
 *                   information about the boot manager
 *                   is written into *pusDisk, *pusPart,
 *                   *BmInfo. Any of these pointers can
 *                   be NULL if you're not interested.
 *
 *      -- ERROR_NOT_SUPPORTED (50): boot manager not installed.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET doshGetBootManager(USHORT   *pusDisk,    // out: if != NULL, boot manager disk (1, 2, ...)
                          USHORT   *pusPart,    // out: if != NULL, index of bmgr primary partition (0-3)
                          PAR_INFO *pBmInfo)    // out: if != NULL, boot manager partition info
{
    APIRET          arc = NO_ERROR;
    USHORT          count = doshQueryDiskCount();    // Physical disk number
    MBR_INFO        MBoot;      // Master Boot
    USHORT          usDisk;

    if (count > 8)              // Not above 8 disks
        count = 8;

    for (usDisk = 1; usDisk <= count; usDisk++)
    {
        USHORT usPrim = 0;

        // for each disk, read the MBR, which has the
        // primary partitions
        if ((arc = doshReadSector(usDisk,
                                  &MBoot,
                                  0,            // head
                                  0,            // cylinder
                                  1)))          // sector
            return arc;

        // scan primary partitions for whether
        // BootManager partition exists
        for (usPrim = 0; usPrim < 4; usPrim++)
        {
            if (MBoot.sPrtnInfo[usPrim].bFileSysCode == 0x0A)
            {
                // this is boot manager:
                if (pBmInfo)
                    *pBmInfo = MBoot.sPrtnInfo[usPrim];
                if (pusPart)
                    *pusPart = usPrim;
                if (pusDisk)
                    *pusDisk = usDisk;
                // stop scanning
                return NO_ERROR;
            }
        }
    }

    return ERROR_NOT_SUPPORTED;
}

/*
 * GetPrimaryPartitions:
 *      this returns the primary partitions.
 *
 *      This gets called from doshGetPartitionsList.
 *
 *      Returns:
 *
 *      -- ERROR_INVALID_PARAMETER: BMInfo is NULL.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 */

STATIC APIRET GetPrimaryPartitions(PARTITIONINFO **pppiFirst,
                                   PARTITIONINFO **pppiThis,
                                   PUSHORT posCount,       // in/out: partition count
                                   PCHAR pcLetter,         // in/out: drive letter counter
                                   UINT BmDisk,            // in: physical disk (1, 2, 3, ...) of boot manager or null
                                   PAR_INFO* pBmInfo,      // in: info returned by doshGetBootManager or NULL
                                   UINT iDisk)             // in: system's physical disk count
{
    APIRET  arc = NO_ERROR;

    if (!pBmInfo)
        arc = ERROR_INVALID_PARAMETER;
    else
    {
        SYS_INFO        MName[32];  // Name Space from Boot Manager
        memset(&MName, 0, sizeof(MName));

        // read boot manager name table;
        // this is in the boot manager primary partition
        // at sector offset 3 (?!?)
        if (!(arc = doshReadSector(BmDisk,
                                   &MName,
                                   // head, cylinder, sector of bmgr primary partition:
                                   pBmInfo->bBeginHead,
                                   GetCyl(pBmInfo->rBeginSecCyl),
                                   GetSec(pBmInfo->rBeginSecCyl) + 3)))
        {
            // got bmgr name table:
            MBR_INFO        MBoot;      // Master Boot
            USHORT          i;

            // read master boot record of this disk
            if (!(arc = doshReadSector(iDisk,
                                       &MBoot,
                                       0,           // head
                                       0,           // cylinder
                                       1)))         // sector
            {
                for (i = 0;
                     i < 4;     // there can be only four primary partitions
                     i++)
                {
                    // skip unused partition, BootManager or Extended partition
                    if (    (MBoot.sPrtnInfo[i].bFileSysCode)  // skip unused
                        &&  (MBoot.sPrtnInfo[i].bFileSysCode != 0x0A) // skip boot manager
                        &&  (MBoot.sPrtnInfo[i].bFileSysCode != 0x05) // skip extended partition
                       )
                    {
                        BOOL fBootable = (    (pBmInfo)
                                           && (MName[(iDisk-1) * 4 + i].bootable & 0x01)
                                         );
                        // store this partition
                        if ((arc = AppendPartition(pppiFirst,
                                                   pppiThis,
                                                   posCount,
                                                   iDisk,
                                                   (fBootable)
                                                     ? (char*)&MName[(iDisk - 1) * 4 + i].name
                                                     : "",
                                                   *pcLetter,
                                                   MBoot.sPrtnInfo[i].bFileSysCode,
                                                   TRUE,        // primary
                                                   fBootable,
                                                   MBoot.sPrtnInfo[i].lTotalSects)))
                            return arc;
                    }
                }
            }
        }
    }

    return arc;
}

/*
 * GetLogicalDrives:
 *      this returns info for the logical drives
 *      in the extended partition. This gets called
 *      from GetExtendedPartition.
 *
 *      This gets called from GetExtendedPartition.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 */

STATIC APIRET GetLogicalDrives(PARTITIONINFO **pppiFirst,
                               PARTITIONINFO **pppiThis,
                               PUSHORT posCount,
                               PCHAR pcLetter,
                               PAR_INFO* PrInfo,                    // in: MBR entry of extended partition
                               UINT PrDisk,
                               PAR_INFO* BmInfo)
{
    APIRET          arc = NO_ERROR;
    EXT_INFO        MBoot;      // Master Boot
    USHORT          i;

    if ((arc = doshReadSector(PrDisk,
                              &MBoot,
                              PrInfo->bBeginHead,
                              GetCyl(PrInfo->rBeginSecCyl),
                              GetSec(PrInfo->rBeginSecCyl))))
        return arc;

    for (i = 0; i < 4; i++)
    {
        // skip unused partition or BootManager partition
        if (    (MBoot.sPrtnInfo[i].bFileSysCode)
             && (MBoot.sPrtnInfo[i].bFileSysCode != 0x0A)
           )
        {
            BOOL    fBootable = FALSE;
            BOOL    fAssignLetter = FALSE;

            // special work around extended partition
            if (MBoot.sPrtnInfo[i].bFileSysCode == 0x05)
            {
                if ((arc = GetLogicalDrives(pppiFirst,
                                            pppiThis,
                                            posCount,
                                            pcLetter,
                                            &MBoot.sPrtnInfo[i],
                                            PrDisk,
                                            BmInfo)))
                    return arc;

                continue;
            }

            // raise driver letter if OS/2 would recognize this drive
            if (    (MBoot.sPrtnInfo[i].bFileSysCode < 0x75)
               )
                fAssignLetter = TRUE;

            if (fAssignLetter)
                (*pcLetter)++;

            fBootable = (   (BmInfo)
                         && ((MBoot.sBmNames[i].bootable & 0x01) != 0)
                        );

            if ((arc = AppendPartition(pppiFirst,
                                       pppiThis,
                                       posCount,
                                       PrDisk,
                                       (fBootable)
                                         ? (char*)&MBoot.sBmNames[i].name
                                         : "",
                                       (fAssignLetter)
                                         ? *pcLetter
                                         : ' ',
                                       MBoot.sPrtnInfo[i].bFileSysCode,
                                       FALSE,        // primary
                                       fBootable,    // bootable
                                       MBoot.sPrtnInfo[i].lTotalSects)))
                return arc;
        }
    }

    return NO_ERROR;
}

/*
 * GetExtendedPartition:
 *      this finds the extended partition on the given
 *      drive and calls GetLogicalDrives in turn.
 *
 *      This gets called from doshGetPartitionsList.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 */

STATIC APIRET GetExtendedPartition(PARTITIONINFO **pppiFirst,
                                   PARTITIONINFO **pppiThis,
                                   PUSHORT posCount,
                                   PCHAR pcLetter,
                                   PAR_INFO* BmInfo,
                                   UINT iDisk)                // in: disk to query
{
    APIRET          arc = NO_ERROR;
    MBR_INFO        MBoot;      // Master Boot
    USHORT          i;

    if ((arc = doshReadSector(iDisk, &MBoot, 0, 0, 1)))
        return arc;

    // go thru MBR entries to find extended partition
    for (i = 0;
         i < 4;
         i++)
    {
        if (MBoot.sPrtnInfo[i].bFileSysCode == 0x05)
        {
            if ((arc = GetLogicalDrives(pppiFirst,
                                        pppiThis,
                                        posCount,
                                        pcLetter,
                                        &MBoot.sPrtnInfo[i],
                                        iDisk,
                                        BmInfo)))
                return arc;
        }
    }

    return NO_ERROR;
}

/*
 *@@ ReadFDiskPartitions:
 *      helper for doshGetPartitionsList for non-LVM
 *      systems.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.16 (2001-10-08) [umoeller]
 */

STATIC APIRET ReadFDiskPartitions(PARTITIONINFO **ppPartitionInfos,
                                  USHORT *pcPartitions,
                                  PUSHORT pusContext)              // out: error context
{
    APIRET          arc = NO_ERROR;

    PAR_INFO        BmInfo;     // BootManager partition
    USHORT          usBmDisk;     // BootManager disk
    USHORT          cDisks = doshQueryDiskCount();    // physical disks count
    USHORT          i;

    CHAR            cLetter = 'C';  // first drive letter

    PARTITIONINFO   *ppiTemp = NULL;

    if (cDisks > 8)              // Not above 8 disks
        cDisks = 8;

    // get boot manager disk and info
    if ((arc = doshGetBootManager(&usBmDisk,
                                  NULL,
                                  &BmInfo)) != NO_ERROR)
    {
        *pusContext = 1;
    }
    else
    {
        // on each disk, read primary partitions
        for (i = 1; i <= cDisks; i++)
        {
            if ((arc = GetPrimaryPartitions(ppPartitionInfos,
                                            &ppiTemp,
                                            pcPartitions,
                                            &cLetter,
                                            usBmDisk,
                                            usBmDisk ? &BmInfo : 0,
                                            i)))
            {
                *pusContext = 2;
            }
        }

        if (!arc && usBmDisk)
        {
            // boot manager found:
            // on each disk, read extended partition
            // with logical drives
            for (i = 1; i <= cDisks; i++)
            {
                if ((arc = GetExtendedPartition(ppPartitionInfos,
                                                &ppiTemp,
                                                pcPartitions,
                                                &cLetter,
                                                &BmInfo,
                                                i)))
                {
                    *pusContext = 3;
                }
            }
        }
    } // end else if ((arc = doshGetBootManager(&usBmDisk,

    return arc;
}

#endif

/*
 *@@ CleanPartitionInfos:
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

STATIC VOID CleanPartitionInfos(PPARTITIONINFO ppiThis)
{
    while (ppiThis)
    {
        PPARTITIONINFO ppiNext = ppiThis->pNext;
        free(ppiThis);
        ppiThis = ppiNext;
    }
}

/*
 *@@ doshGetPartitionsList:
 *      this returns lots of information about the
 *      partitions on all physical disks, which is
 *      read directly from the MBRs and partition
 *      tables.
 *
 *      If NO_ERROR is returned by this function,
 *      *ppPartitionInfo points to a linked list of
 *      PARTITIONINFO structures, which has
 *      *pusPartitionCount items.
 *
 *      In that case, use doshFreePartitionsList to
 *      free the resources allocated by this function.
 *
 *      What this function returns depends on whether
 *      LVM is installed.
 *
 *      --  If LVM.DLL is found on the LIBPATH, this opens
 *          the LVM engine and returns the info from the
 *          LVM engine in the PARTITIONINFO structures.
 *          The partitions are then sorted by disk in
 *          ascending order.
 *
 *      --  Otherwise, we parse the partition tables
 *          manually. The linked list then starts out with
 *          all the primary partitions, followed by the
 *          logical drives in the extended partitions.
 *          This function attempts to guess the correct drive
 *          letters and stores these with the PARTITIONINFO
 *          items, but there's no guarantee that this is
 *          correct. We correctly ignore Linux partitions here
 *          and give all primary partitions the C: letter, but
 *          I have no idea what happens with NTFS partitions,
 *          since I have none.
 *
 *      If an error != NO_ERROR is returned, *pusContext
 *      will be set to one of the following:
 *
 *      --  1: boot manager not found
 *
 *      --  2: primary partitions error
 *
 *      --  3: secondary partitions error
 *
 *      --  0: something else.
 *
 *      Originally contributed by Dmitry A. Steklenev.
 *
 *@@added V0.9.0 [umoeller]
 *@@changed V0.9.9 (2001-04-07) [umoeller]: added transparent LVM support; changed prototype
 *@@changed V0.9.9 (2001-04-07) [umoeller]: fixed memory leaks on errors
 */

APIRET doshGetPartitionsList(PPARTITIONSLIST *ppList,
                             PUSHORT pusContext)                // out: error context
{
    APIRET          arc = NO_ERROR;

    PLVMINFO        pLVMInfo = NULL;

    PARTITIONINFO   *pPartitionInfos = NULL; // linked list of all partitions
    USHORT          cPartitions = 0;        // bootable partition count

    if (!ppList)
        return ERROR_INVALID_PARAMETER;

    if (!(arc = doshQueryLVMInfo(&pLVMInfo)))
    {
        // LVM installed:
        arc = doshReadLVMPartitions(pLVMInfo,         // in: LVM info
                                    &pPartitionInfos, // out: partitions array
                                    &cPartitions);      // out: partitions count
        // copied to output below

        if (arc)
        {
            // error: start over
            doshFreeLVMInfo(pLVMInfo);
            CleanPartitionInfos(pPartitionInfos);
            pPartitionInfos = NULL;
            cPartitions = 0;
        }
    }

#ifndef __XWPLITE__
    if (arc)
        // LVM not installed, or failed:
        // parse partitions manually
        arc = ReadFDiskPartitions(&pPartitionInfos,
                                  &cPartitions,
                                  pusContext);
#endif

    if (!arc)
    {
        // no error so far:
        *pusContext = 0;

        *ppList = NEW(PARTITIONSLIST);
        if (!(*ppList))
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            ZERO(*ppList);

            (*ppList)->pPartitionInfo = pPartitionInfos;
            (*ppList)->cPartitions = cPartitions;

            _Pmpf((__FUNCTION__ ": returning %d partitions", cPartitions));
        }
    }

    if (arc)
        CleanPartitionInfos(pPartitionInfos);

    _Pmpf((__FUNCTION__ ": exiting, arc = %d", arc));

    return arc;
}

/*
 *@@ doshFreePartitionsList:
 *      this frees the resources allocated by
 *      doshGetPartitionsList.
 *
 *@@added V0.9.0 [umoeller]
 */

APIRET doshFreePartitionsList(PPARTITIONSLIST ppList)
{
    if (!ppList)
        return ERROR_INVALID_PARAMETER;

    CleanPartitionInfos(ppList->pPartitionInfo);
    doshFreeLVMInfo(ppList->pLVMInfo);
    free(ppList);

    return NO_ERROR;
}

/********************************************************************
 *
 *   LVM declarations
 *
 ********************************************************************/

/*
 *@@category: Helpers\Control program helpers\Partitions info\Quick LVM Interface
 *      functions for transparently interfacing LVM.DLL.
 */

typedef unsigned char       BOOLEAN;
typedef unsigned short int  CARDINAL16;
typedef unsigned long       CARDINAL32;
typedef unsigned int        CARDINAL;
typedef unsigned long       DoubleWord;

#ifdef ADDRESS
#undef ADDRESS
#endif

typedef void* ADDRESS;

#pragma pack(1)

#define DISK_NAME_SIZE          20
#define FILESYSTEM_NAME_SIZE    20
#define PARTITION_NAME_SIZE     20
#define VOLUME_NAME_SIZE        20

/*
 *@@ Drive_Control_Record:
 *      invariant for a disk drive.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

typedef struct _Drive_Control_Record
{
    CARDINAL32   Drive_Number;                      // OS/2 Drive Number for this drive.
    CARDINAL32   Drive_Size;                        // The total number of sectors on the drive.
    DoubleWord   Drive_Serial_Number;               // The serial number assigned to this drive.  For info. purposes only.
    ADDRESS      Drive_Handle;                      // Handle used for operations on the disk that this record corresponds to.
    CARDINAL32   Cylinder_Count;                    // The number of cylinders on the drive.
    CARDINAL32   Heads_Per_Cylinder;                // The number of heads per cylinder for this drive.
    CARDINAL32   Sectors_Per_Track;                 // The number of sectors per track for this drive.
    BOOLEAN      Drive_Is_PRM;                      // Set to TRUE if this drive is a PRM.
    BYTE         Reserved[3];                       // Alignment.
} Drive_Control_Record;

/*
 *@@ Drive_Control_Array:
 *      returned by the Get_Drive_Control_Data function
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

typedef struct _Drive_Control_Array
{
    Drive_Control_Record *   Drive_Control_Data;    // An array of drive control records.
    CARDINAL32               Count;                 // The number of entries in the array of drive control records.
} Drive_Control_Array;

/*
 *@@ Drive_Information_Record:
 *      defines the information that can be changed for a specific disk drive.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

typedef struct _Drive_Information_Record
{
    CARDINAL32   Total_Available_Sectors;           // The number of sectors on the disk which are not currently assigned to a partition.
    CARDINAL32   Largest_Free_Block_Of_Sectors;     // The number of sectors in the largest contiguous block of available sectors.
    BOOLEAN      Corrupt_Partition_Table;           // If TRUE, then the partitioning information found on the drive is incorrect!
    BOOLEAN      Unusable;                          // If TRUE, the drive's MBR is not accessible and the drive can not be partitioned.
    BOOLEAN      IO_Error;                          // If TRUE, then the last I/O operation on this drive failed!
    BOOLEAN      Is_Big_Floppy;                     // If TRUE, then the drive is a PRM formatted as a big floppy (i.e. the old style removable media support).
    char         Drive_Name[DISK_NAME_SIZE];        // User assigned name for this disk drive.
} Drive_Information_Record;

/*
 *@@ Partition_Information_Record:
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

typedef struct _Partition_Information_Record
{
    ADDRESS      Partition_Handle;
            // The handle used to perform operations on this partition.
    ADDRESS      Volume_Handle;
            // If this partition is part of a volume, this will be the handle of
            // the volume.  If this partition is NOT part of a volume, then this
            // handle will be 0.
    ADDRESS      Drive_Handle;
            // The handle for the drive this partition resides on.
    DoubleWord   Partition_Serial_Number;
            // The serial number assigned to this partition.
    CARDINAL32   Partition_Start;
            // The LBA of the first sector of the partition.
    CARDINAL32   True_Partition_Size;
            // The total number of sectors comprising the partition.
    CARDINAL32   Usable_Partition_Size;
            // The size of the partition as reported to the IFSM.  This is the
            // size of the partition less any LVM overhead.
    CARDINAL32   Boot_Limit;
            // The maximum number of sectors from this block of free space that
            // can be used to create a bootable partition if you allocate from the
            // beginning of the block of free space.
    BOOLEAN      Spanned_Volume;
            // TRUE if this partition is part of a multi-partition volume.
    BOOLEAN      Primary_Partition;
            // True or False.  Any non-zero value here indicates that this partition
            // is a primary partition.  Zero here indicates that this partition is
            // a "logical drive" - i.e. it resides inside of an extended partition.
    BYTE         Active_Flag;
            // 80 = Partition is marked as being active.
            // 0 = Partition is not active.
    BYTE         OS_Flag;
            // This field is from the partition table.  It is known as the OS flag,
            // the Partition Type Field, Filesystem Type, and various other names.
            //      Values of interest
            //      If this field is: (values are in hex)
            //      07 = The partition is a compatibility partition formatted for use
            //      with an installable filesystem, such as HPFS or JFS.
            //      00 = Unformatted partition
            //      01 = FAT12 filesystem is in use on this partition.
            //      04 = FAT16 filesystem is in use on this partition.
            //      0A = OS/2 Boot Manager Partition
            //      35 = LVM partition
            //      84 = OS/2 FAT16 partition which has been relabeled by Boot Manager to "Hide" it.
    BYTE         Partition_Type;
            // 0 = Free Space
            // 1 = LVM Partition (Part of an LVM Volume.)
            // 2 = Compatibility Partition
            // All other values are reserved for future use.
    BYTE         Partition_Status;
            // 0 = Free Space
            // 1 = In Use - i.e. already assigned to a volume.
            // 2 = Available - i.e. not currently assigned to a volume.
    BOOLEAN      On_Boot_Manager_Menu;
            // Set to TRUE if this partition is not part of a Volume yet is on the
            // Boot Manager Menu.
    BYTE         Reserved;
            // Alignment.
    char         Volume_Drive_Letter;
            // The drive letter assigned to the volume that this partition is a part of.
    char         Drive_Name[DISK_NAME_SIZE];
            // User assigned name for this disk drive.
    char         File_System_Name[FILESYSTEM_NAME_SIZE];
            // The name of the filesystem in use on this partition, if it is known.
    char         Partition_Name[PARTITION_NAME_SIZE];
            // The user assigned name for this partition.
    char         Volume_Name[VOLUME_NAME_SIZE];
            // If this partition is part of a volume, then this will be the
            // name of the volume that this partition is a part of.  If this
            // record represents free space, then the Volume_Name will be
            // "FREE SPACE xx", where xx is a unique numeric ID generated by
            // LVM.DLL.  Otherwise it will be an empty string.
} Partition_Information_Record;

// The following defines are for use with the Partition_Type field in the
// Partition_Information_Record.
#define FREE_SPACE_PARTITION     0
#define LVM_PARTITION            1
#define COMPATIBILITY_PARTITION  2

// The following defines are for use with the Partition_Status field in the
// Partition_Information_Record.
#define PARTITION_IS_IN_USE      1
#define PARTITION_IS_AVAILABLE   2
#define PARTITION_IS_FREE_SPACE  0

/*
 *@@ Partition_Information_Array:
 *      returned by various functions in the LVM Engine.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

typedef struct _Partition_Information_Array
{
    Partition_Information_Record * Partition_Array; // An array of Partition_Information_Records.
    CARDINAL32                     Count;           // The number of entries in the Partition_Array.
} Partition_Information_Array;

/*
 *@@ Volume_Information_Record:
 *      variable information for a volume.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

typedef struct _Volume_Information_Record
{
    CARDINAL32 Volume_Size;
            // The number of sectors comprising the volume.
    CARDINAL32 Partition_Count;
            // The number of partitions which comprise this volume.
    CARDINAL32 Drive_Letter_Conflict;
            // 0   indicates that the drive letter preference for this volume is unique.
            // 1   indicates that the drive letter preference for this volume
            //       is not unique, but this volume got its preferred drive letter anyway.
            // 2   indicates that the drive letter preference for this volume
            //       is not unique, and this volume did NOT get its preferred drive letter.
            // 4   indicates that this volume is currently "hidden" - i.e. it has
            //       no drive letter preference at the current time.
    BOOLEAN    Compatibility_Volume;
            // TRUE if this is for a compatibility volume, FALSE otherwise.
    BOOLEAN    Bootable;
            // Set to TRUE if this volume appears on the Boot Manager menu, or if it is
            // a compatibility volume and its corresponding partition is the first active
            // primary partition on the first drive.
    char       Drive_Letter_Preference;
            // The drive letter that this volume desires to be.
    char       Current_Drive_Letter;
            // The drive letter currently used to access this volume.
            // May be different than Drive_Letter_Preference if there was a conflict ( i.e. Drive_Letter_Preference
            // is already in use by another volume ).
    char       Initial_Drive_Letter;
            // The drive letter assigned to this volume by the operating system
            // when LVM was started. This may be different from the
            // Drive_Letter_Preference if there were conflicts, and
            // may be different from the Current_Drive_Letter.  This
            // will be 0x0 if the Volume did not exist when the LVM Engine
            // was opened (i.e. it was created during this LVM session).
    BOOLEAN    New_Volume;
            // Set to FALSE if this volume existed before the LVM Engine was
            // opened.  Set to TRUE if this volume was created after the LVM
            // Engine was opened.
    BYTE       Status;
            // 0 = None.
            // 1 = Bootable
            // 2 = Startable
            // 3 = Installable.
    BYTE       Reserved_1;
    char       Volume_Name[VOLUME_NAME_SIZE];
            // The user assigned name for this volume.
    char       File_System_Name[FILESYSTEM_NAME_SIZE];
            // The name of the filesystem in use on this partition, if it
            // is known.
} Volume_Information_Record;

#pragma pack()

/********************************************************************
 *
 *   Quick LVM Interface API
 *
 ********************************************************************/

/*
 *@@ LVMINFOPRIVATE:
 *      private structure used by doshQueryLVMInfo.
 *      This is what the LVMINFO pointer really
 *      points to.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

typedef struct _LVMINFOPRIVATE
{
    LVMINFO             LVMInfo;            // public structure (dosh.h)

    // function pointers resolved from LVM.DLL

    void (* _System     Open_LVM_Engine)(BOOLEAN Ignore_CHS,
                                         CARDINAL32 *Error_Code);

    void (* _System     Free_Engine_Memory)(ADDRESS Object);

    void (* _System     Close_LVM_Engine)(void);

    Drive_Control_Array (* _System
                        Get_Drive_Control_Data)(CARDINAL32 *Error_Code);

    Drive_Information_Record (* _System
                        Get_Drive_Status)(ADDRESS Drive_Handle,
                                          CARDINAL32 *Error_Code);

    Partition_Information_Array (* _System
                        Get_Partitions)(ADDRESS Handle,
                                        CARDINAL32 *Error_Code);

    Volume_Information_Record (*_System
                        Get_Volume_Information)(ADDRESS Volume_Handle,
                                                CARDINAL32 *Error_Code);

} LVMINFOPRIVATE, *PLVMINFOPRIVATE;

#define LVM_ERROR_FIRST             20000

/*
 *@@ doshQueryLVMInfo:
 *      creates an LVMINFO structure if LVM is installed.
 *      Returns that structure (which the caller must free
 *      using doshFreeLVMInfo) or NULL if LVM.DLL was not
 *      found along the LIBPATH.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

APIRET doshQueryLVMInfo(PLVMINFO *ppLVMInfo)
{
    APIRET          arc = NO_ERROR;
    CHAR            szError[100];
    PLVMINFOPRIVATE pLVMInfo = NULL;
    HMODULE         hmodLVM = NULLHANDLE;

    if (!(arc = DosLoadModule(szError,
                              sizeof(szError),
                              "LVM",
                              &hmodLVM)))
    {
        // got LVM.DLL:
        pLVMInfo = NEW(LVMINFOPRIVATE);
        if (!pLVMInfo)
            arc = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            // array of function pointers to be resolved from LVM.DLL
            RESOLVEFUNCTION     aFunctions[7];
            ULONG               ul;

            aFunctions[0].pcszFunctionName="Open_LVM_Engine";
			aFunctions[0].ppFuncAddress=(PFN*)&pLVMInfo->Open_LVM_Engine;
			
            aFunctions[1].pcszFunctionName="Free_Engine_Memory";
			aFunctions[1].ppFuncAddress=(PFN*)&pLVMInfo->Free_Engine_Memory;
			
            aFunctions[2].pcszFunctionName="Close_LVM_Engine";
			aFunctions[2].ppFuncAddress=(PFN*)&pLVMInfo->Close_LVM_Engine;
			
            aFunctions[3].pcszFunctionName="Get_Drive_Control_Data";
			aFunctions[3].ppFuncAddress=(PFN*)&pLVMInfo->Get_Drive_Control_Data;
			
            aFunctions[4].pcszFunctionName="Get_Drive_Status";
			aFunctions[4].ppFuncAddress=(PFN*)&pLVMInfo->Get_Drive_Status;
			
            aFunctions[5].pcszFunctionName="Get_Partitions";
			aFunctions[5].ppFuncAddress=(PFN*)&pLVMInfo->Get_Partitions;
			
            aFunctions[6].pcszFunctionName="Get_Volume_Information";
			aFunctions[6].ppFuncAddress=(PFN*)&pLVMInfo->Get_Volume_Information;

            ZERO(pLVMInfo);

            pLVMInfo->LVMInfo.hmodLVM = hmodLVM;

            // now resolve function pointers
            for (ul = 0;
                 ul < ARRAYITEMCOUNT(aFunctions);
                 ul++)
            {
                PRESOLVEFUNCTION pFuncThis = &aFunctions[ul];
                arc = DosQueryProcAddr(hmodLVM,
                                       0,               // ordinal, ignored
                                       (PSZ)pFuncThis->pcszFunctionName,
                                       pFuncThis->ppFuncAddress);
                if (!pFuncThis->ppFuncAddress)
                    arc = ERROR_INVALID_NAME;

                if (arc)
                    break;
            }
        }
    }

    if (arc)
        doshFreeLVMInfo((PLVMINFO)pLVMInfo);
    else
        *ppLVMInfo = (PLVMINFO)pLVMInfo;

    return arc;
}

/*
 *@@ doshReadLVMPartitions:
 *      using the LVMINFO parameter from doshQueryLVMInfo,
 *      builds an array of PARTITIONINFO structures with
 *      the data returned from LVM.DLL.
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

APIRET doshReadLVMPartitions(PLVMINFO pInfo,         // in: LVM info
                             PPARTITIONINFO *ppPartitionInfo, // out: partitions array
                             PUSHORT pcPartitions)      // out: partitions count
{
    APIRET          arc = NO_ERROR;
    CARDINAL32      Error = 0;

    PARTITIONINFO   *pPartitionInfos = NULL, // linked list of all partitions
                    *ppiTemp = NULL;
    USHORT          cPartitions = 0;        // bootable partition count
    PLVMINFOPRIVATE pLVMInfo = (PLVMINFOPRIVATE)pInfo;

    _Pmpf((__FUNCTION__ ": entering"));

    if (!pLVMInfo)
        return ERROR_INVALID_PARAMETER;

    // initialize LVM engine
    pLVMInfo->Open_LVM_Engine(TRUE,
                              &Error);

    _Pmpf(("  Open_LVM_Engine Error: %d", Error));

    if (!Error)
    {
        Drive_Control_Array DCA = pLVMInfo->Get_Drive_Control_Data(&Error);
                        // member records to be freed

        _Pmpf(("  Get_Drive_Control_Data Error: %d, drive count: %d", Error, DCA.Count));

        if (    (!Error)
             && (DCA.Count)
           )
        {
            // DCA.Drive_Control_Data now contains drive information records;
            // this must be freed
            ULONG   ulDisk;

            for (ulDisk = 0;
                 ulDisk < DCA.Count;
                 ulDisk++)
            {
                Drive_Control_Record *pDriveControlRecord
                    = &DCA.Drive_Control_Data[ulDisk];
                ADDRESS hDrive = pDriveControlRecord->Drive_Handle;

                /* Drive_Information_Record pDriveInfoRecord
                    = pLVMInfo->Get_Drive_Status(hDrive,
                                                 &Error);

                _Pmpf(("  drive %d Get_Drive_Status Error: %d", ulDisk, Error));

                if (!Error) */
                {
                    Partition_Information_Array PIA
                        = pLVMInfo->Get_Partitions(hDrive,
                                                   &Error);

                    _Pmpf(("    Get_Partitions Error: %d", Error));

                    if (!Error)
                    {
                        // PIA.Partition_Array now contains
                        // Partition_Information_Record; must be freed

                        // now go thru partitions of this drive
                        ULONG ulPart;
                        for (ulPart = 0;
                             ulPart < PIA.Count;
                             ulPart++)
                        {
                            Partition_Information_Record *pPartition
                                = &PIA.Partition_Array[ulPart];
                            Volume_Information_Record VolumeInfo;

                            const char  *pcszBootName = NULL; // for now
                            BOOL        fBootable = FALSE;

                            if (pPartition->Volume_Handle)
                            {
                                // this partition is part of a volume:
                                // only then can it be bootable...
                                // get the volume info
                                VolumeInfo
                                    = pLVMInfo->Get_Volume_Information(pPartition->Volume_Handle,
                                                                       &Error);
                                pcszBootName = VolumeInfo.Volume_Name;

                                fBootable = (VolumeInfo.Status == 1);
                            }


                            if (arc = AppendPartition(&pPartitionInfos,
                                                      &ppiTemp,
                                                      &cPartitions,
                                                      ulDisk + 1,
                                                      pcszBootName,
                                                      pPartition->Volume_Drive_Letter,
                                                      pPartition->OS_Flag,  // FS type
                                                      pPartition->Primary_Partition,
                                                      fBootable,
                                                      pPartition->True_Partition_Size))
                                break;
                        }

                        // clean up partitions
                        pLVMInfo->Free_Engine_Memory(PIA.Partition_Array);
                    }
                }
                /* else
                    // error:
                    break; */
            }

            // clean up drive data
            pLVMInfo->Free_Engine_Memory(DCA.Drive_Control_Data);
        }
    }

    // close LVM
    pLVMInfo->Close_LVM_Engine();

    if (Error)
    {
        // if we got an error, return it with the
        // LVM error offset
        arc = LVM_ERROR_FIRST + Error;

        CleanPartitionInfos(pPartitionInfos);
    }

    if (!arc)
    {
        *ppPartitionInfo = pPartitionInfos;
        *pcPartitions = cPartitions;
    }

    _Pmpf((__FUNCTION__ ": exiting, arg = %d", arc));

    return arc;
}

/*
 *@@ doshFreeLVMInfo:
 *
 *@@added V0.9.9 (2001-04-07) [umoeller]
 */

VOID doshFreeLVMInfo(PLVMINFO pInfo)
{
    if (pInfo)
    {
        if (pInfo->hmodLVM)
            DosFreeModule(pInfo->hmodLVM);

        free(pInfo);
    }
}

/*
 *@@category: Helpers\Control program helpers\Wildcard matching
 *      See doshMatch.
 */

/* ******************************************************************
 *
 *   Wildcard matching
 *
 ********************************************************************/

/*
 * PerformMatch:
 *      compares a single path component. The input strings must
 *      not have slashes or backslashes in them.
 *
 *      fHasDot must be true if pName contains at least one dot.
 *
 *      Note that this function is recursive.
 */

STATIC BOOL PerformMatch(PCSZ pMask,
                         PCSZ pName,
                         int fHasDot)
{
    while (TRUE)
    {
        // go thru the pMask char by char
        switch (*pMask)
        {
            case 0:
                // if we've reached the end of the mask,
                // we better have the end of the name too
                if (*pName == 0)
                    return TRUE;
                return FALSE;

            case '?':
                // a question mark matches one single character;
                // it does _not_ match a dot;
                // at the end of the component, it also matches
                // no characters
                if (    (*pName != '.')
                     && (*pName != 0)
                   )
                    ++pName;
                ++pMask;
            break;

            case '*':
                // asterisk matches zero or more characters

                // skip extra asterisks
                /*
                do
                {
                    ++pMask;
                } while (*pMask == '*');
                */

                while (*(++pMask) == '*')       // V0.9.20 (2002-07-25) [umoeller]
                    ;

                // pMask points to after '*';
                // pName is unchanged... so for each pName
                // that follows, check if it matches
                while (TRUE)
                {
                    if (PerformMatch(pMask, pName, fHasDot))
                        // the remainder matched:
                        // then everything matches
                        return TRUE;

                    if (*pName == 0)
                        return FALSE;

                    // didn't match: try next pName
                    ++pName;
                }

            case '.':
                // a dot matches a dot only, even if the name doesn't
                // have one at the end
                ++pMask;
                if (*pName == '.')
                    ++pName;
                else if (    (fHasDot)
                          || (*pName != 0)
                        )
                    return FALSE;
            break;

            default:
                if (*pMask++ != *pName++)
                    return FALSE;
            break;
        }
    }
}

/*
 *@@ doshMatchCase:
 *      this matches '*' and '?' wildcards, similar to what
 *      DosEditName does. However, this does not require a
 *      file to be present, but works on strings only.
 *
 *      Returns TRUE if the given name matches the given mask.
 *
 *      This accepts both short and fully qualified masks and
 *      names, but the following rules apply:
 *
 *      --  Either both the mask and the name must be fully
 *          qualified, or both must not. Otherwise the match fails.
 *
 *      --  If fully qualified, only the last component may contain
 *          wildcards.
 *
 *      --  This compares WITH respect to case always. Upper-case
 *          both the mask and the name before calling this, or
 *          use doshMatch instead.
 *
 *      --  As opposed to the WPS, this handles multiple dots in
 *          filenames correctly. For example, the WPS will not
 *          match "*.ZIP" against "whatever-0.9.3.zip", but this
 *          one will.
 *
 *      This replaces strhMatchOS2 which has been removed with
 *      V0.9.16 and is a lot faster than the old code, which has
 *      been completely rewritten.
 *
 *@@added V0.9.16 (2002-01-01) [umoeller]
 */

BOOL doshMatchCase(const char *pcszMask,     // in: mask (e.g. "*.TXT")
                   const char *pcszName)     // in: string to check (e.g. "TEST.TXT")
{
    BOOL    brc = FALSE;

    PCSZ    pLastMaskComponent,
            pLastNameComponent;

    ULONG   cbMaskPath = 0,
            cbNamePath = 0;

    if (pLastMaskComponent = strrchr(pcszMask, '\\'))
    {
        // length of path component
        cbMaskPath = pLastMaskComponent - pcszMask;
        pLastMaskComponent++;
    }
    else
        pLastMaskComponent = pcszMask;

    if (pLastNameComponent = strrchr(pcszName, '\\'))
    {
        // length of path component
        cbNamePath = pLastNameComponent - pcszName;
        pLastNameComponent++;
    }
    else
        pLastNameComponent = pcszName;

    // compare paths; if the lengths are different
    // or memcmp fails, we can't match
    if (    (cbMaskPath == cbNamePath)      // can both be null
         && (    (cbMaskPath == 0)
              || (!memcmp(pcszMask, pcszName, cbMaskPath))
            )
       )
    {
        // alright, paths match:
        brc = PerformMatch(pLastMaskComponent,
                           pLastNameComponent,
                           // has dot?
                           (strchr(pLastNameComponent, '.') != NULL));

    }

    return brc;
}

/*
 *@@ doshMatchCaseNoPath:
 *      like doshMatchCase, but is faster if you are sure that
 *      neither pcszMask nor pcszName contain path separators
 *      ("\" characters). In other words, this is for short
 *      filenames.
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

BOOL doshMatchCaseNoPath(const char *pcszMask,     // in: mask (e.g. "*.TXT")
                         const char *pcszName)     // in: string to check (e.g. "TEST.TXT")
{
    return PerformMatch(pcszMask,
                        pcszName,
                        // has dot?
                        (strchr(pcszName, '.') != NULL));
}

/*
 *@@ doshMatch:
 *      like doshMatchCase, but compares without respect
 *      to case.
 *
 *@@added V0.9.16 (2002-01-26) [umoeller]
 */

BOOL doshMatch(const char *pcszMask,     // in: mask (e.g. "*.TXT")
               const char *pcszName)     // in: string to check (e.g. "TEST.TXT")
{
    ULONG   cbMask = strlen(pcszMask),
            cbName = strlen(pcszName);
    PSZ     pszMask = (PSZ)_alloca(cbMask + 1),
            pszName = (PSZ)_alloca(cbName + 1);

    memcpy(pszMask, pcszMask, cbMask + 1);
    nlsUpper(pszMask);
    memcpy(pszName, pcszName, cbName + 1);
    nlsUpper(pszName);

    return doshMatchCase(pszMask,
                         pszName);
}
