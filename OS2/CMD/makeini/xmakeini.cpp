/*
 *  xmakeini.c:
 *      source code for xmakeini.exe, a replacement for
 *      MAKEINI that can handle more than one RC file
 *      for input and string replacements.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INCL_DOSERRORS
#include <os2.h>

#include "setup.h"
#include "bldlevel.h"

#include "linklist.h"
#include "standards.h"
#include "stringh.h"
#include "xprf.h"
#include "xstring.h"

/*
 *@@ SEDENTRY:
 *
 */

typedef struct _SEDENTRY
{
    XSTRING strFind,
            strReplace;

} SEDENTRY, *PSEDENTRY;

LINKLIST    G_llSeds;
XSTRING     G_strEsc1,
            G_strEsc2;
ULONG       G_ulVerbosity = 1;      // 0 = dots and errors only
                                    // 1 = default
                                    // 2 = talkative

/*
 *@@ ReplaceEscapes:
 *
 */

VOID ReplaceEscapes(PXSTRING pstr)
{
    ULONG   ulOfs = 0;
    size_t  ShiftTable[256];
    BOOL    fRepeat = FALSE;
    while (xstrFindReplace(pstr,
                           &ulOfs,
                           &G_strEsc1,
                           &G_strEsc2,
                           ShiftTable,
                           &fRepeat))
    {
        if (G_ulVerbosity >= 2)
            printf("   replaced \"%s\" with \"%s\" at ofs %d\n",
                   G_strEsc1.psz,
                   G_strEsc2.psz,
                   ulOfs);
    }
}

/*
 *@@ LoadSedFile:
 *
 */

APIRET LoadSedFile(PCSZ pcszSedFilename)
{
    APIRET  arc = NO_ERROR;
    FILE    *SedFile;
    if (!(SedFile = fopen(pcszSedFilename, "r")))
    {
        arc = -1;
        printf("Cannot open sedfile \"%s\".\n",
               pcszSedFilename);
    }
    else
    {
        ULONG   ulLine = 0;
        CHAR    szLineBuf[4000];

        while (!arc)
        {
            if (!(fgets(szLineBuf, sizeof(szLineBuf), SedFile)))
            {
                if (!(feof(SedFile)))
                    arc = _doserrno;

                break;
            }

            // we only support simple sed syntax, that is
            // s/find/replace/ and the rest of the line
            // will be ignored; besides, no regexps, only
            // plain text searches

            ulLine++;

            if (    (szLineBuf[0] == '\n')
                 || (szLineBuf[0] == '\r')
               )
                // empty line:
                continue;

            if (    (szLineBuf[0] != 's')
                 && (szLineBuf[1] != '/')
               )
            {
                printf("Error in line %d of \"%s\": Line does not start with \"/s\".\n",
                       pcszSedFilename,
                       ulLine);
                arc = -1;
                break;
            }

            PCSZ pEndOfFind;
            if (!(pEndOfFind = strchr(szLineBuf + 2, '/')))
            {
                printf("Error in line %d of \"%s\": Line has no '/' character after \"/s\".\n",
                       pcszSedFilename,
                       ulLine);
                arc = -1;
                break;
            }

            PCSZ pEndOfReplace;
            if (!(pEndOfReplace = strchr(pEndOfFind + 1, '/')))
            {
                printf("Error in line %d of \"%s\": Line has no '/' character after find expression.\n",
                       pcszSedFilename,
                       ulLine);
                arc = -1;
                break;
            }

            PSEDENTRY pSed;
            if (!(pSed = NEW(SEDENTRY)))
            {
                arc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            // s/find/replace/
            // 012
            //       ^ pEndOfFind
            //               ^ pEndOfReplace

            xstrInit(&pSed->strFind, 0);
            xstrcpy(&pSed->strFind,
                    szLineBuf + 2,
                    pEndOfFind - (szLineBuf + 2));

            ReplaceEscapes(&pSed->strFind);

            xstrInit(&pSed->strReplace, 0);
            xstrcpy(&pSed->strReplace,
                    pEndOfFind + 1,
                    pEndOfReplace - (pEndOfFind + 1));

            ReplaceEscapes(&pSed->strReplace);

            lstAppendItem(&G_llSeds,
                          pSed);
        }

        fclose(SedFile);
    }

    return arc;
}

/*
 *@@ ApplyRCFile:
 *
 */

APIRET ApplyRCFile(PXINI pIni,
                   PCSZ pcszRCFilename,
                   PULONG pcKeysAdded)
{
    APIRET  arc = NO_ERROR;
    FILE    *RCFile;
    if (!(RCFile = fopen(pcszRCFilename, "r")))
    {
        arc = -1;
        printf("Cannot open rc file \"%s\".\n",
               pcszRCFilename);
    }
    else
    {
        ULONG   ulLine = 0;
        CHAR    szLineBuf[4000];
        XSTRING strLine;
        xstrInit(&strLine, 0);

        while (!arc)
        {
            if (!(fgets(szLineBuf, sizeof(szLineBuf), RCFile)))
            {
                if (!(feof(RCFile)))
                    arc = _doserrno;

                break;
            }

            // we only support simple sed syntax, that is
            // s/find/replace/ and the rest of the line
            // will be ignored; besides, no regexps, only
            // plain text searches

            ulLine++;

            if (    (szLineBuf[0] == '\n')
                 || (szLineBuf[0] == '\r')
               )
                // empty line:
                continue;

            PLISTNODE pNode;
            xstrcpy(&strLine, szLineBuf, 0);
            FOR_ALL_NODES(&G_llSeds, pNode)
            {
                PSEDENTRY pSed = (PSEDENTRY)pNode->pItemData;
                ULONG   ulOfs = 0;
                size_t  ShiftTable[256];
                BOOL    fRepeat = FALSE;
                while (xstrFindReplace(&strLine,
                                       &ulOfs,
                                       &pSed->strFind,
                                       &pSed->strReplace,
                                       ShiftTable,
                                       &fRepeat))
                {
                    if (G_ulVerbosity >= 2)
                        printf("\n   line %d: replaced \"%s\" with \"%s\" at ofs %d",
                               ulLine,
                               pSed->strFind.psz,
                               pSed->strReplace.psz,
                               ulOfs);
                }
            }

            // finally go parse the line

            // 1) app name (first pair of quotes)
            PSZ pStartOfApp = strLine.psz;
            while (*pStartOfApp == ' ')
                pStartOfApp++;

            if (    (*pStartOfApp != '\"')
                 || (pStartOfApp[1] == '\"')
               )
                // probably comment, just ignore
                continue;

            PSZ pEndOfApp;
            if (!(pEndOfApp = strchr(pStartOfApp + 2, '\"')))
            {
                printf("Error in line %d of \"%s\": Invalid quote order.\n",
                        ulLine,
                        pcszRCFilename);
                arc = -1;
                break;
            }

            PSZ pszApp = pStartOfApp + 1;
            *pEndOfApp = '\0';

            // 2) key name (first pair of quotes)
            PSZ pStartOfKey = pEndOfApp + 1;
            while (*pStartOfKey == ' ')
                pStartOfKey++;

            if (    (*pStartOfKey != '\"')
                 || (pStartOfKey[1] == '\"')
               )
            {
                printf("Error in line %d of \"%s\": Invalid key name after application.\n",
                        ulLine,
                        pcszRCFilename);
                arc = -1;
                break;
            }

            PSZ pEndOfKey;
            if (!(pEndOfKey = strchr(pStartOfKey + 2, '\"')))
            {
                printf("Error in line %d of \"%s\": Invalid quote order.\n",
                        ulLine,
                        pcszRCFilename);
                arc = -1;
                break;
            }

            PSZ pszKey = pStartOfKey + 1;
            *pEndOfKey = '\0';

            // 3) value (third part)
            PSZ pStartOfValue = pEndOfKey + 1;
            while (*pStartOfValue == ' ')
                pStartOfValue++;

            // now this can be different...
            PBYTE   pbValue = NULL;
            ULONG   cbValue = 0;
            PSZ     pEndOfValue;
            BOOL    fFreeValue = FALSE;
            LONG    lValue;

            if (    (*pStartOfValue >= '0')
                 && (*pStartOfValue <= '9')
               )
            {
                // decimal, octal, or hex value:
                lValue = strtol(pStartOfValue + 2,
                                NULL,
                                0);

                pbValue = (PBYTE)&lValue;
                cbValue = sizeof(LONG);
            }
            else if (    (*pStartOfValue == '\"')
                      && (pStartOfValue[1] == '\"')
                    )
            {
                // "" means just create key, so allocate a null
                // byte for the value
                lValue = 0;
                pbValue = (PBYTE)&lValue;
                cbValue = 1;
            }
            else
            {
                PSZ pEndOfValue;
                if (!(pEndOfValue = strchr(pStartOfValue + 2, '\"')))
                {
                    printf("Error in line %d of \"%s\": Invalid quote order.\n",
                            ulLine,
                            pcszRCFilename);
                    arc = -1;
                    break;
                }

                pbValue = pStartOfValue + 1;
                *pEndOfValue = '\0';
                cbValue = pEndOfValue - pStartOfValue; // include null byte
            }

            if (!(arc = xprfWriteProfileData(pIni,
                                             pszApp,
                                             pszKey,
                                             pbValue,
                                             cbValue)))
                (*pcKeysAdded)++;

            if (arc || (G_ulVerbosity >= 2))
                printf("xprfWriteProfileData returned %d for app \"%s\", key \"%s\", %d bytes of data (%s)\n",
                       arc,
                       pszApp,
                       pszKey,
                       cbValue,
                       pIni->szFilename);
        }

        fclose(RCFile);
    }

    return arc;
}

/*
 *@@ main:
 *
 *      Syntax: xmakeini <inifile> <rcfile> [<rcfile> ...] [-s sedfile]
 */

int main(int argc, char* argv[])
{
    APIRET  arc = NO_ERROR;
    if (argc < 3)
    {
        printf("xmakeini V" BLDLEVEL_VERSION " built " __DATE__ "\n");
        printf("(C) 2002 Ulrich Moeller\n");
        printf("Licensed to Serenity Systems International\n");
        printf("Syntax: xmakeini <inifile> [-s sedfile] [-d <dir>] <rcfile> [<rcfile> ...]\n");
        printf("with <inifile>: the INI file to create or update; must not be in use!\n");
        printf("     -s <sedfile>: a file in very simply sed syntax with string replacements\n");
        printf("        to apply on the rc file where each line must be \"s/find/replace/\"\n");
        printf("        (regular expressions are not yet supported)\n");
        printf("     -d <dir>: a directory specification where to find the rc files (to\n");
        printf("        keep the command line short\n");
        printf("     <rcfile>: the resource script to apply (there can be several)\n");
        exit(-1);
    }

    lstInit(&G_llSeds, FALSE);

    xstrInitCopy(&G_strEsc1, "\\\\", 0);
    xstrInitCopy(&G_strEsc2, "\\", 0);

    // open the profile file
    PXINI pIni;
    if (arc = xprfOpenProfile(argv[1],
                              &pIni))
    {
        printf("Error %d occured opening \"%s\"\n", arc, argv[1]);
    }
    else
    {
        ULONG   cKeysTotal = 0;
        CHAR    szDirectory[CCHMAXPATH] = "";

        int i = 2;
        while (i < argc)
        {
            ULONG   cKeysThis = 0;

            if (!strcmp(argv[i], "-q"))
            {
                G_ulVerbosity = 0;
            }
            else if (!strcmp(argv[i], "-v"))
            {
                G_ulVerbosity = 2;
            }
            else if (!strcmp(argv[i], "-d"))
            {
                strcpy(szDirectory,
                       argv[++i]);
            }
            else if (!strcmp(argv[i], "-s"))
            {
                if (i + 1 < argc)
                {
                    ++i;
                    if (G_ulVerbosity)
                        printf("Loading sed file \"%s\"\n",
                               argv[i]);

                    if (arc = LoadSedFile(argv[i]))
                        printf("!! LoadSedFile returned %d\n",
                               arc);
                }
                else
                {
                    printf("Error: Missing argument after -s.\n");
                    arc = -1;
                }
            }
            else
            {
                CHAR    szFile[CCHMAXPATH];
                PSZ     pszFile;
                if (szDirectory[0])
                {
                    strcpy(szFile, szDirectory);
                    if (szFile[strlen(szFile) - 1] != '\\')
                        strcat(szFile, "\\");
                    strcat(szFile, argv[i]);
                    pszFile = szFile;
                }
                else
                    pszFile = argv[i];

                if (!G_ulVerbosity)
                    printf(".");
                else
                {
                    printf("Applying \"%s\" on \"%s\"... ",
                           pszFile,
                           argv[1]);
                }
                fflush(stdout);

                if (arc = ApplyRCFile(pIni,
                                      pszFile,
                                      &cKeysThis))
                    printf("!! ApplyRCFile returned %d\n",
                           arc);

                if (G_ulVerbosity)
                    printf("%d keys written\n", cKeysThis);

                cKeysTotal += cKeysThis;
            }

            if (arc)
                break;

            i++;
        }

        if (!arc && G_ulVerbosity)
            printf("%d keys written in total.\n", cKeysTotal);

        if (G_ulVerbosity >= 2)
            printf("pIni->fDirty is %d\n", pIni->fDirty);

        xprfCloseProfile(pIni);
    }

    return arc;
}
