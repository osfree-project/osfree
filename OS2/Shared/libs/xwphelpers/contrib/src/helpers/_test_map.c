
#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\linklist.h"
#include "helpers\mapfile.h"
#include "helpers\stringh.h"
#include "helpers\standards.h"

#pragma hdrstop

/*
 *@@ main:
 *
 */

int main (int argc, char *argv[])
{
    int         rc;
    PMAPFILE    pMapFile;
    ULONG       ulLineError = 0;

    CHAR        szLineBuf[2000] = "";

    if (argc < 2)
    {
        printf("Usage: map <mapfile>\n");
        exit(1);
    }

    if (!(rc = mapvRead(argv[1],
                        &pMapFile,
                        szLineBuf,
                        sizeof(szLineBuf),
                        &ulLineError)))
    {
        PLISTNODE pNode;

        printf("Map file for module \"%s\" successfully read\n",
               pMapFile->szModule);

        printf("Found %d segments\n", lstCountItems(&pMapFile->llSegments));
        FOR_ALL_NODES(&pMapFile->llSegments, pNode)
        {
            PSEGMENT pSeg = (PSEGMENT)pNode->pItemData;
            printf("  \"%20s\" %04lX:%08lX (0x%lX bytes)\n",
                   pSeg->szSegName,
                   pSeg->ulObjNo,
                   pSeg->ulObjOfs,
                   pSeg->cbSeg);
        }

        printf("Found %d exports\n", lstCountItems(&pMapFile->llExports));
        FOR_ALL_NODES(&pMapFile->llExports, pNode)
        {
            PEXPORT pExp = (PEXPORT)pNode->pItemData;
            printf("  %04lX:%08lX \"%s\" \"%s\"\n",
                   pExp->ulObjNo,
                   pExp->ulObjOfs,
                   pExp->szNames,
                   pExp->szNames + pExp->ulName1Len + 1);
        }

        printf("Found %d public symbols\n", lstCountItems(&pMapFile->llPublics));
        FOR_ALL_NODES(&pMapFile->llPublics, pNode)
        {
            PPUBLIC pPub = (PPUBLIC)pNode->pItemData;
            if (pPub->ulImportLen)
                printf("  %04lX:%08lX \"%s\" (%s.%d)\n",
                       pPub->ulObjNo,
                       pPub->ulObjOfs,
                       pPub->szName,
                       pPub->szName + pPub->ulNameLen + 1,
                       pPub->ulImportIndex);
            else
                printf("  %04lX:%08lX \"%s\"\n",
                       pPub->ulObjNo,
                       pPub->ulObjOfs,
                       pPub->szName);
        }

        mapvClose(&pMapFile);
    }
    else
    {
        printf("Error %d occurred at line %d\n", rc, ulLineError);
        printf("Line buf: \"%s\"", szLineBuf);
    }

    return rc;
}



