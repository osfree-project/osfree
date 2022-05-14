#define INCL_KBD

#include <os2.h>

int main(VOID)
{
PVOID     Data;    /*  Pointer to event data. */
PULONG    Kind;    /*  Kind of event returned. */
APIRET    rc;  /*  Return code. */

rc = KbdGetConsole(Data, Kind, Flag, hkbd);
return 0;
}