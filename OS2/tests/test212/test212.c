#define INCL_KBD

#include <os2.h>

int main (VOID)
{

PKBDKEYINFO    CharData;  /*  Pointer to character data. */

APIRET         rc;        /*  Return code. */



rc = KbdCharIn(CharData, Wait, hkbd);

return 0;
}