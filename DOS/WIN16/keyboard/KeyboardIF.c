#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define NOAPIPROTO
#include "windows.h"
#include "Willows.h"

#include "WinDefs.h"
#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "Resources.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "ModTable.h"

int ToAscii(UINT wVirtKey, UINT wScanCode, BYTE FAR* lpKeyState, DWORD FAR* lpChar, UINT wFlags);

void
IT_TOASCII(ENV *envp,LONGPROC f)
{
        LPBYTE lpStruct;
        DWORD dwTransKey;
        DWORD retcode;

        lpStruct = (LPBYTE)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
        retcode = (DWORD)ToAscii((UINT)GETWORD(SP+16),
                                (UINT)GETWORD(SP+14),
                                lpStruct,
                                &dwTransKey,
                                (UINT)GETWORD(SP+4));
        lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
        PUTDWORD(lpStruct,dwTransKey);
        envp->reg.sp += 3*UINT_86 + 2*LP_86 + RET_86;
        envp->reg.ax = LOWORD(retcode);
        envp->reg.dx = HIWORD(retcode);
}

