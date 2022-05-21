
#define INCL_KBD

#include <os2.h>

int main(VOID)
{

USHORT     *name;  /*  Keyboard layout name. */

APIRET      rc;    /*  Return code. */



rc = KbdGetLayoutUni(name, hkbd);

return 0;
}
