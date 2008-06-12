#define INCL_DOSMISC
#include <os2.h>

#include "setup.h"                      // code generation and debugging options

#include "dosh.h"
#include "level.h"

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
