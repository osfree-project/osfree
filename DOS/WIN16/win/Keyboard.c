
#include <string.h>
#include "windows.h"

#include "Log.h"
#include "Driver.h"

void WINAPI
GetKeyboardState(BYTE FAR *lpbKeyState)
{
    if (lpbKeyState)
	(void)DRVCALL_KEYBOARD(PKH_KBSTATE,0,0,lpbKeyState);
}
	
void WINAPI
SetKeyboardState(BYTE FAR *lpbKeyState)
{
    if (lpbKeyState)
	(void)DRVCALL_KEYBOARD(PKH_KBSTATE,1,0,lpbKeyState);
}

int WINAPI
GetKeyState(int nVirtKey)
{
    int retcode;

    retcode = (int)DRVCALL_KEYBOARD(PKH_KEYSTATE,nVirtKey,0,0);

    APISTR((LF_API,"GetKeyState(key=%x) returns %x\n",nVirtKey,retcode));

    return retcode;
}

int WINAPI
GetAsyncKeyState(int nVirtKey)
{
    int retcode;
    static UINT AsyncKeyState[256];

    retcode = (int)DRVCALL_KEYBOARD(PKH_KEYSTATE,nVirtKey,1,0);

    if (AsyncKeyState[nVirtKey] == 0 && retcode != 0)
	retcode |= 1;

    AsyncKeyState[nVirtKey] = 1;

    APISTR((LF_API,"GetAsyncKeyState(key=%x) returns %x\n",nVirtKey,retcode));

    return retcode;
}

