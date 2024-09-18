#define INCL_PMWP
#include <os2.h>

typedef struct {
   EXCEPTIONREGISTRATIONRECORD Exc;
   ULONG rEBP;
   ULONG rEBX;
   ULONG rEDI;
   ULONG rESI;
   ULONG rESP;
   ULONG ret;
} SHLEXCEPTIONREGISTRATIONRECORD, *PSHLEXCEPTIONREGISTRATIONRECORD;


APIRET APIENTRY ShlSaveEnv(PEXCEPTIONREGISTRATIONRECORD pERegRec, PVOID handler)
{
	return 0;
}


APIRET APIENTRY ShlRestoreEnv(PEXCEPTIONREGISTRATIONRECORD pERegRec)
{
	__asm {
;		mov ecx, pERegRec
;		mov ebx, [ecx+xEBX]
	}
	return 0;
}

APIRET APIENTRY ShlStartWorkplace(HAB hab, HMQ hmq)
{
	return 0;
}

APIRET APIENTRY MessageLoopProc(HAB hab)
{
  QMSG   qmsg;
  HWND   hwnd;

  for (;;)
  {
    while (WinGetMsg(hab, &qmsg, 0, 0, 0))
      WinDispatchMsg(hab, &qmsg);

    hwnd = (HWND)qmsg.mp2;

    if (((HWND)qmsg.msg == WM_QUIT) && (WinIsWindow(hab, hwnd)))
      WinSendMsg(hwnd, (ULONG)WM_SYSCOMMAND, (MPARAM)SC_CLOSE, (MPARAM)CMDSRC_MENU);
  }

  return 0;
}

APIRET APIENTRY ShlExceptionHandler(void)
{
  return 0;
}
