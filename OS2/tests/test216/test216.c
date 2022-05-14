#define INCL_KBD

#include <os2.h>

int main(VOID)
{
PKBDHWID    pkbdhwid;  /*  Pointer to hardware ID. */
APIRET      rc;    /*  Return code. */

rc = KbdGetHWID(pkbdhwid, hkbd);
return 0;
}
