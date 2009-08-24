
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

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

/* the following are the MMSYSTEM interfaces */
extern DWORD make_native_thunk(DWORD, DWORD);

void
IT_TIMEGETDEVCAPS (ENV *envp,LONGPROC f)
{
	TIMECAPS tc;
	DWORD retcode;
	LPBYTE lpStruct;

	if ((retcode = timeGetDevCaps(&tc,(UINT)GETWORD(SP+4))) == 0 &&
	    (lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6)))) {
	    PUTWORD(lpStruct,(WORD)tc.wPeriodMin);
	    PUTWORD(lpStruct+2,(WORD)tc.wPeriodMax);
	}
	envp->reg.sp += LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_TIMEGETSYSTEMTIME (ENV *envp,LONGPROC f)
{
	MMTIME mmt;
	DWORD retcode;
	LPBYTE lpStruct;

	if ((retcode = timeGetSystemTime(&mmt,(UINT)GETWORD(SP+4))) == 0 &&
	    (lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6)))) {
	    PUTWORD(lpStruct,(WORD)mmt.wType);
	    PUTDWORD(lpStruct+2,mmt.u.ms);
	}
	envp->reg.sp += LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_TIMESETEVENT (ENV *envp,LONGPROC f)	/* timeSetEvent */
{
	DWORD retcode;
	DWORD dwProc;

	dwProc = GETDWORD(SP+10);
	dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_mmtimer):0L;
	retcode = timeSetEvent((UINT)GETWORD(SP+16),
			(UINT)GETWORD(SP+14),
			(LPTIMECALLBACK)dwProc,
			GETDWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 3*UINT_86 + LP_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}


