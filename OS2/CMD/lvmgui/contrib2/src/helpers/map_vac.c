/*
 *@@sourcefile map_vac.c:
 *      map file parser for the VAC 3.08 format.
 *
 *      Usage: All OS/2 programs.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@added V1.0.0 (2002-09-13) [umoeller]
 *
 *@@header "helpers\mapfile.h"
 */

/*
 *      This file Copyright (C) 2002 Ulrich M”ller.
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

#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DONT_REPLACE_MALLOC
#include "setup.h"                      // code generation and debugging options

#include "helpers\linklist.h"
#include "helpers\mapfile.h"
#include "helpers\stringh.h"
#include "helpers\standards.h"

#pragma hdrstop

/*
 *@@category: Helpers\Control program helpers\Map file parsing (VAC 3.08)
 *      See map_vac.c.
 */

/*
 *@@ MAPSTATE:
 *
 */

typedef enum _MAPSTATE
{
    MAP_START,
            // until we find " Start         Length     Name                   Class"

    MAP_SEGMENTS,
            // each segment is introduced by
            // " 0001:00000000 000001630H FREQ_CODE1             CODE 32-bit"
            // then for each source file
            // "    at offset 00000000 003A4H bytes from T:\_build\xwp\xfdataf.obj (xfdataf.c)"

            // until we find " Origin   Group"

    MAP_GROUPS,
            // whatever this is for, ignoring for now
            // until we find " Address       Export                  Alias"

    MAP_EXPORTS,
            // for each export
            // " 0003:00096AE8 apmhClose               apmhClose"

            // until we find
            // "  Address         Publics by Name"

    MAP_PUBLICS_BY_NAME,
            // for each public
            // " 0003:000A800A       anmBlowUpBitmap"
            // or, for imports,
            // " 0000:00000000  Imp  DevCloseDC           (PMGPI.604)"

            // until we find
            // "  Address         Publics by Value"

    MAP_PUBLICS_BY_VALUE,
            // same thing as previous section, we can ignore that

            // until we find
            // "Line numbers for T:\_build\xwp\xcenter.obj(xcenter.c)"

    MAP_LINENUMBERS_FILEINTRO,

            // starts with:
            // "    Record Number of Start of Source: 0"
            // "    Number of Primary Source Records: 0"
            // "    Source & Listing Files: 67"

            // then
            // "    File  1) xcenter.c" etc.

            // until we find
            // "Line numbers for T:\_build\xwp\xcenter.obj(xcenter.c) segment CODE32 (@0x00000000)"

    MAP_LINENUMBERS_FILEDATA,

            // starts with:
            // "     Source    Src File    Seg:Offset
            // "    Line Num    Index          (0X)
            // "    --------   --------   -------------

            // then for each line
            // "      1630          1     0003:00000000"

            // until we find again
            // "Line numbers for T:\_build\xwp\xcenter.obj(xcenter.c)"

            // or
            // "Program entry point at 0003:00023228"

} MAPSTATE;

/*
 *@@ mapvRead:
 *
 */

APIRET mapvRead(PCSZ pcszMapFilename,
                PMAPFILE *ppMapFile,
                PSZ pszLineBuf,
                ULONG cbLineBuf,
                PULONG pulLineError)        // out: line no. where we choked
{
    APIRET      arc = NO_ERROR;
    ULONG       ulLine = 2;

    MAPSTATE    State = MAP_START;

    PMAPFILE    pMapFile = NULL;
    PSEGMENT    pCurrentSegment = NULL;

    FILE        *MapFile;

    if (!(MapFile = fopen(pcszMapFilename, "r")))
        return _doserrno;

    while (!arc)
    {
        if (!(fgets(pszLineBuf, cbLineBuf, MapFile)))
        {
            if (!(feof(MapFile)))
                arc = _doserrno;

            break;
        }

        // printf("Running line \"%s\"", pszLineBuf);

        ulLine++;

        if (    (pszLineBuf[0] == '\n')
             || (pszLineBuf[0] == '\r')
           )
            // empty line:
            continue;

        #define STRNCMP(p1, p2) memcmp(p1, p2, sizeof(p2) - 1)

        switch (State)
        {
            case MAP_START:
                if (!STRNCMP(pszLineBuf,
                             " Start         Length     Name                   Class"))
                {
                    if (!pMapFile)
                        arc = MAPERROR_NO_MODULE_NAME;
                    else
                        State = MAP_SEGMENTS;
                }
                else
                {
                    PSZ p = pszLineBuf;
                    while (*p == ' ')
                        p++;
                    if (*p)
                    {
                        if (!(pMapFile = NEW(MAPFILE)))
                            arc = ERROR_NOT_ENOUGH_MEMORY;
                        else
                        {
                            ZERO(pMapFile);
                            strncpy(pMapFile->szModule,
                                    p,
                                    strchr(p, '\n') - p);

                            lstInit(&pMapFile->llSegments, TRUE);
                            lstInit(&pMapFile->llExports, TRUE);
                            lstInit(&pMapFile->llPublics, TRUE);
                        }
                    }
                    else
                        return MAPERROR_NO_MODULE_NAME;
                }
            break;

            case MAP_SEGMENTS:
                if (!STRNCMP(pszLineBuf,
                             " Origin   Group"))
                {
                    pCurrentSegment = NULL;

                    State = MAP_GROUPS;
                }
                else if (!STRNCMP(pszLineBuf,
                         "\tat offset "))
                {

                }
                // " 0001:00000000 000001630H FREQ_CODE1             CODE 32-bit"
                else if (    (pszLineBuf[0] == ' ')
                          && (pszLineBuf[5] == ':')
                        )
                {
                    if (!(pCurrentSegment = NEW(SEGMENT)))
                        arc = ERROR_NOT_ENOUGH_MEMORY;
                    else
                    {
                        ZERO(pCurrentSegment);

                        if (3 != sscanf(pszLineBuf,
                                        " %04lX:%08lX %08lXH ",
                                        &pCurrentSegment->ulObjNo,
                                        &pCurrentSegment->ulObjOfs,
                                        &pCurrentSegment->cbSeg))
                        {
                            arc = MAPERROR_SYNTAX_SEGMENTS_START;
                        }
                        else
                        {
                            PSZ pStartOfSegName = pszLineBuf + sizeof(" 0001:00000000 000001630H ") - 1;
                            PSZ pSpaceAfterSegName;

                            if (!(pSpaceAfterSegName = strchr(pStartOfSegName, ' ')))
                                arc = MAPERROR_SYNTAX_SEGMENTS_NOSEGNAME;
                            else
                            {
                                PSZ pStartOfSegType;
                                memcpy(pCurrentSegment->szSegName,
                                       pStartOfSegName,
                                       pSpaceAfterSegName - pStartOfSegName);

                                pStartOfSegType = pszLineBuf + sizeof(" 0006:00000000 00001E288H CONST32_RO             ") - 1;
                                if (!STRNCMP(pStartOfSegType,
                                             "CODE 32-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_CODE32;
                                else if (!STRNCMP(pStartOfSegType,
                                                  "CODE 16-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_CODE16;
                                else if (!STRNCMP(pStartOfSegType,
                                                  "CONST 32-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_CONST32;
                                else if (!STRNCMP(pStartOfSegType,
                                                  "DATA16 16-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_DATA16;
                                else if (!STRNCMP(pStartOfSegType,
                                                  "DATA 32-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_DATA32;
                                else if (!STRNCMP(pStartOfSegType,
                                                  "EDC_CLASS 32-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_EDC_CLASS32;
                                else if (!STRNCMP(pStartOfSegType,
                                                  "BSS 32-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_BSS32;
                                else if (!STRNCMP(pStartOfSegType,
                                                  "STACK 32-bit"))
                                    pCurrentSegment->ulType = SEGTYPE_STACK32;
                                else
                                {
                                    printf("  MAP_SEGMENTS: Cannot handle segment type \"%s\"",
                                           pStartOfSegType);
                                    arc = MAPERROR_SYNTAX_SEGMENTS_SEGTYPE;
                                }
                            }

                            if (!arc)
                                lstAppendItem(&pMapFile->llSegments,
                                              pCurrentSegment);
                            else
                                FREE(pCurrentSegment);
                        }
                    }
                }
                else
                    arc = MAPERROR_SYNTAX_SEGMENTS_OTHER;
            break;

            case MAP_GROUPS:
                if (!STRNCMP(pszLineBuf,
                             " Address       Export                  Alias"))
                {
                    State = MAP_EXPORTS;
                }
                // executables usually have no exports, so we
                // might run into the publics as well
                else if (!STRNCMP(pszLineBuf,
                             "  Address         Publics by Name"))
                {
                    State = MAP_PUBLICS_BY_NAME;
                }
            break;

            case MAP_EXPORTS:
                if (!STRNCMP(pszLineBuf,
                             "  Address         Publics by Name"))
                {
                    State = MAP_PUBLICS_BY_NAME;
                }
                // " 0003:00096AE8 apmhClose               apmhClose"
                else if (    (pszLineBuf[0] == ' ')
                          && (pszLineBuf[5] == ':')
                        )
                {
                    EXPORT  expTemp;

                    if (2 != sscanf(pszLineBuf,
                                    " %04lX:%08lX ",
                                    &expTemp.ulObjNo,
                                    &expTemp.ulObjOfs))
                        arc = MAPERROR_SYNTAX_EXPORTS_OBJOFS;
                    else
                    {
                        PSZ pStartOfName1 = pszLineBuf + sizeof(" 0003:00096AE8 ") - 1,
                            pEndOfName1;
                        if (!(pEndOfName1 = strchr(pStartOfName1, ' ')))
                            arc = MAPERROR_SYNTAX_EXPORTS_NAME1;
                        else
                        {
                            PSZ pStartOfName2 = pEndOfName1,
                                pEndOfName2;

                            PEXPORT pExport;

                            while (*pStartOfName2 == ' ')
                                pStartOfName2++;
                            pEndOfName2 = strhFindEOL(pStartOfName2, NULL);

                            expTemp.ulName1Len = pEndOfName1 - pStartOfName1;
                            expTemp.ulName2Len = pEndOfName2 - pStartOfName2;

                            if (!(pExport = malloc(   sizeof(EXPORT)
                                                    + expTemp.ulName1Len
                                                    + expTemp.ulName2Len
                                                    + 1
                                                  )))
                                arc = ERROR_NOT_ENOUGH_MEMORY;
                            else
                            {
                                PSZ pszName2;
                                memcpy(pExport,
                                       &expTemp,
                                       sizeof(expTemp));
                                memcpy(pExport->szNames,
                                       pStartOfName1,
                                       expTemp.ulName1Len);
                                pExport->szNames[expTemp.ulName1Len] = '\0';

                                pszName2 = pExport->szNames +expTemp.ulName1Len + 1;
                                memcpy(pszName2,
                                       pStartOfName2,
                                       expTemp.ulName2Len);
                                pszName2[expTemp.ulName2Len] = '\0';

                                lstAppendItem(&pMapFile->llExports, pExport);
                            }
                        }
                    }
                }
                else
                    arc = MAPERROR_SYNTAX_EXPORTS_OTHER;
            break;

            case MAP_PUBLICS_BY_NAME:
                if (!STRNCMP(pszLineBuf,
                             "  Address         Publics by Value"))
                {
                    State = MAP_PUBLICS_BY_VALUE;
                }
                // " 0003:00096AE8 apmhClose               apmhClose"
                else if (    (pszLineBuf[0] == ' ')
                          && (pszLineBuf[5] == ':')
                        )
                {
                    PUBLIC  pubTemp;

                    if (2 != sscanf(pszLineBuf,
                                    " %04lX:%08lX ",
                                    &pubTemp.ulObjNo,
                                    &pubTemp.ulObjOfs))
                        arc = MAPERROR_SYNTAX_PUBLICS_OBJOFS;
                    else
                    {
                        PSZ     p2 = pszLineBuf + sizeof(" 0000:00000000  ") - 1,
                                p3;
                        PPUBLIC pPublic;
                        PCSZ    pcszImportLib = NULL;

                        BOOL    fImport = !STRNCMP(p2, "Imp");

                        p2 += 5;

                        pubTemp.ulImportLen = 0;
                        pubTemp.ulImportIndex = 0;

                        if (fImport)
                        {
                            p3 = strchr(p2, ' ');
                            pubTemp.ulNameLen = p3 - p2;
                            if (p3 = strchr(p3, '('))
                            {
                                PSZ p4;
                                pcszImportLib = p3 + 1;
                                p4 = strchr(pcszImportLib, '.');
                                // "(DOSCALLS.303)"
                                pubTemp.ulImportLen = p4 - pcszImportLib;
                                pubTemp.ulImportIndex = atoi(p4 + 1);
                            }
                        }
                        else
                            // no import:
                            p3 = strhFindEOL(p2, &pubTemp.ulNameLen);

                        if (!(pPublic = malloc(   sizeof(PUBLIC)
                                                + pubTemp.ulNameLen
                                                + pubTemp.ulImportLen
                                                + 1)))
                            arc = ERROR_NOT_ENOUGH_MEMORY;
                        else
                        {
                            memcpy(pPublic, &pubTemp, sizeof(PUBLIC));
                            memcpy(pPublic->szName,
                                   p2,
                                   pubTemp.ulNameLen);
                            pPublic->szName[pubTemp.ulNameLen] = '\0';

                            if (pubTemp.ulImportLen)
                            {
                                PSZ pszImport = pPublic->szName + pubTemp.ulNameLen + 1;
                                memcpy(pszImport,
                                       pcszImportLib,
                                       pubTemp.ulImportLen);
                                pszImport[pubTemp.ulImportLen] = '\0';
                            }


                            lstAppendItem(&pMapFile->llPublics, pPublic);
                        }
                    }
                }
                else
                    arc = MAPERROR_SYNTAX_PUBLICS_OTHER;
            break;

            case MAP_PUBLICS_BY_VALUE:
                if (!STRNCMP(pszLineBuf,
                             "Line numbers for "))
                {
                    // "Line numbers for T:\_build\xwp\xcenter.obj(xcenter.c)"
                    PSZ pObjStart,
                        pSourceStart,
                        pSourceEnd;

    StartLineNumersIntro:

                    pObjStart = pszLineBuf + sizeof("Line numbers for ") - 1;
                    if (pSourceStart = strchr(pObjStart, '('))
                    {
                        *pSourceStart++ = '\0';
                        if (pSourceEnd = strchr(pSourceStart, ')'))
                            *pSourceEnd = '\0';
                    }

                    // starts with:
                    // "    Record Number of Start of Source: 0"
                    // "    Number of Primary Source Records: 0"
                    // "    Source & Listing Files: 67"


                    State = MAP_LINENUMBERS_FILEINTRO;
                }
            break;

            case MAP_LINENUMBERS_FILEINTRO:
                if (!STRNCMP(pszLineBuf,
                             "Line numbers for "))
                {
                    State = MAP_LINENUMBERS_FILEDATA;
                }
            break;

            case MAP_LINENUMBERS_FILEDATA:
                if (!STRNCMP(pszLineBuf,
                             "Program entry point at "))
                {
                    break;
                }
                else if (!STRNCMP(pszLineBuf,
                                  "Line numbers for "))
                {
                    goto StartLineNumersIntro;
                }
            break;

            default:
                printf("Internal error, unknown parsing state.\n");
            break;
        }
    }

    if (arc)
    {
        mapvClose(&pMapFile);
        *pulLineError = ulLine;
    }
    else
        *ppMapFile = pMapFile;

    fclose(MapFile);

    return arc;
}

/*
 *@@ mapvClose:
 *
 */

APIRET mapvClose(PMAPFILE *ppMapFile)
{
    PMAPFILE pMapFile;

    if (    (!ppMapFile)
         || (!(pMapFile = *ppMapFile))
       )
        return ERROR_INVALID_PARAMETER;

    // these three are auto-free
    lstClear(&pMapFile->llSegments);
    lstClear(&pMapFile->llExports);
    lstClear(&pMapFile->llPublics);

    free(pMapFile);
    *ppMapFile = NULL;
}
