/*!
 * @file main.c
 * @brief Command-line entry point for the HC30 compiler.
 */
#include "os2types.h"
#include "os2err.h"
#include "rtf.h"
#include "hpj.h"
#include "hfs.h"
#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    HHPJ    hHpj = NULLHANDLE;
    HRTFDOC hDoc = NULLHANDLE;
    HHFS    hHfs = NULLHANDLE;
    APIRET  rc;
    ULONG   ulFileCount = 0, ulIdx;
    int     iStatus = 0;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s project.hpj output.hlp\n", argv[0]);
        return 1;
    }

    if ((rc = HpjCreateDoc(&hHpj)) != NO_ERROR) {
        fprintf(stderr, "HpjCreateDoc: %lu\n", (unsigned long)rc);
        return 1;
    }
    if ((rc = RtfCreateDoc(&hDoc)) != NO_ERROR) {
        fprintf(stderr, "RtfCreateDoc: %lu\n", (unsigned long)rc);
        HpjDestroyDoc(hHpj);
        return 1;
    }

    if ((rc = HpjReadFile(hHpj, argv[1])) != NO_ERROR) {
        fprintf(stderr, "HpjReadFile(%s): %lu\n", argv[1], (unsigned long)rc);
        iStatus = 1;
        goto cleanup;
    }

    /* Activate build tags. */
    {
        char szTag[128];
        PCSZ apszTags[64];
        ULONG cTags = 0, i, n = 0;
        HpjQueryBuildTagCount(hHpj, &n);
        for (i = 0; i < n && cTags < 64; i++) {
            szTag[0] = '\0';
            if (HpjQueryBuildTagName(hHpj, i, szTag, sizeof(szTag), NULL)
                != NO_ERROR)
                continue;
            if (szTag[0] == '-') continue;
            {
                const char* p = szTag;
                char* pcopy;
                if (*p == '+') p++;
                if (!*p) continue;
                pcopy = strdup(p);
                if (pcopy) apszTags[cTags++] = pcopy;
            }
        }
        RtfSetBuildTags(hDoc, cTags, apszTags);
        for (i = 0; i < cTags; i++) free((void*)apszTags[i]);
    }

    if (HpjQueryFileCount(hHpj, &ulFileCount) == NO_ERROR) {
        for (ulIdx = 0; ulIdx < ulFileCount; ulIdx++) {
            char szFile[512];
            szFile[0] = '\0';
            if (HpjQueryFileName(hHpj, ulIdx, szFile,
                                 sizeof(szFile), NULL) != NO_ERROR)
                continue;
            if ((rc = RtfReadFile(hDoc, szFile)) != NO_ERROR) {
                fprintf(stderr, "RtfReadFile(%s): %lu\n",
                        szFile, (unsigned long)rc);
                iStatus = 1;
                goto cleanup;
            }
        }
    }

    if ((rc = CmpCompile(hHpj, hDoc, argv[1], &hHfs)) != NO_ERROR) {
        fprintf(stderr, "CmpCompile: %lu\n", (unsigned long)rc);
        iStatus = 1;
        goto cleanup;
    }

    if ((rc = HfsWrite(hHfs, argv[2])) != NO_ERROR) {
        fprintf(stderr, "HfsWrite(%s): %lu\n", argv[2], (unsigned long)rc);
        iStatus = 1;
        goto cleanup;
    }

cleanup:
    if (hHfs != NULLHANDLE) HfsDestroy(hHfs);
    if (hDoc != NULLHANDLE) RtfDestroyDoc(hDoc);
    if (hHpj != NULLHANDLE) HpjDestroyDoc(hHpj);
    return iStatus;
}
