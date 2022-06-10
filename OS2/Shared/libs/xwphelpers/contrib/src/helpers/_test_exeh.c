
#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSESMGR
#define INCL_DOSQUEUES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_KBD
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\exeh.h"
#include "helpers\standards.h"

#pragma hdrstop

int main (int argc, char *argv[])
{
    int arc = 0;
    PEXECUTABLE pExe;

    if (argc != 2)
    {
        printf("exeh (built " __DATE__ "): displays the bldlevel of an executable.\n");
        printf("Usage: exeh <name.exe>\n");
        arc = 1003;
    }
    else
    {
        if (!(arc = exehOpen(argv[1], &pExe)))
        {
            APIRET  arc2;
            ULONG   progt;

            printf("exeh: dumping base info of \"%s\"\n", argv[1]);
            printf("    cbDosExeHeader    %d\n", pExe->cbDosExeHeader);
            printf("    ulExeFormat       %d (%s)\n",
                    pExe->ulExeFormat,
                    (pExe->ulExeFormat == EXEFORMAT_OLDDOS) ? "EXEFORMAT_OLDDOS"
                    : (pExe->ulExeFormat == EXEFORMAT_NE) ? "EXEFORMAT_NE"
                    : (pExe->ulExeFormat == EXEFORMAT_PE) ? "EXEFORMAT_PE"
                    : (pExe->ulExeFormat == EXEFORMAT_LX) ? "EXEFORMAT_LX"
                    : (pExe->ulExeFormat == EXEFORMAT_TEXT_BATCH) ? "EXEFORMAT_TEXT_BATCH"
                    : (pExe->ulExeFormat == EXEFORMAT_TEXT_CMD) ? "EXEFORMAT_TEXT_CMD"
                    : (pExe->ulExeFormat == EXEFORMAT_COM) ? "EXEFORMAT_COM"
                    : "unknown"
                  );
            if (pExe->pLXHeader)
                printf("    LX flags:         0x%lX\n", pExe->pLXHeader->ulFlags);
            else if (pExe->pNEHeader)
                printf("    NE flags:         0x%lX\n", pExe->pNEHeader->usFlags);
            printf("    fLibrary          %d\n", pExe->fLibrary);
            printf("    f32Bits           %d\n", pExe->f32Bits);
            if (!(arc2 = exehQueryProgType(pExe, &progt)))
                printf("    progtype:         %d (%s)\n", progt, exehDescribeProgType(progt));
            else
                printf("    exehQueryProgType returned %d\n", arc2);
            printf("exeh: dumping bldlevel of \"%s\"\n", argv[1]);

            if (!(arc = exehQueryBldLevel(pExe)))
            {
                #define STRINGORNA(p) ((pExe->p) ? (pExe->p) : "n/a")

                printf("    Description:      \"%s\"\n", STRINGORNA(pszDescription));
                printf("    Vendor:           \"%s\"\n", STRINGORNA(pszVendor));
                printf("    Version:          \"%s\"\n", STRINGORNA(pszVersion));
                printf("    Info:             \"%s\"\n", STRINGORNA(pszInfo));
                printf("    Build date/time:  \"%s\"\n", STRINGORNA(pszBuildDateTime));
                printf("    Build machine:    \"%s\"\n", STRINGORNA(pszBuildMachine));
                printf("    ASD:              \"%s\"\n", STRINGORNA(pszASD));
                printf("    Language:         \"%s\"\n", STRINGORNA(pszLanguage));
                printf("    Country:          \"%s\"\n", STRINGORNA(pszCountry));
                printf("    Revision:         \"%s\"\n", STRINGORNA(pszRevision));
                printf("    Unknown string:   \"%s\"\n", STRINGORNA(pszUnknown));
                printf("    Fixpak:           \"%s\"\n", STRINGORNA(pszFixpak));

            }

            exehClose(&pExe);
        }

        if (arc)
        {
            printf("exeh: Error %d occurred with \"%s\".\n", arc, argv[1]);
        }
    }

    return arc;
}
