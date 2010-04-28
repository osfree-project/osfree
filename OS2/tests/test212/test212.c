#define INCL_KBD

#include <os2.h>



PKBDKEYINFO    CharData;  /*  Pointer to character data. */

APIRET         rc;        /*  Return code. */



rc = KbdCharIn(CharData, Wait, hkbd);
