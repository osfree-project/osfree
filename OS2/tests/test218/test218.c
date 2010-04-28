
#define INCL_KBD

#include <os2.h>



USHORT     *name;  /*  Keyboard layout name. */

APIRET      rc;    /*  Return code. */



rc = KbdGetLayoutUni(name, hkbd);
