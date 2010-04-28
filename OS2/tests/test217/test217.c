#define INCL_KBD

#include <os2.h>


APIRET    rc;       /*  Return code. */



rc = KbdGetLayout(pszName, hkbd);
