#define INCL_KBD

#include <os2.h>


PUSHORT    pidCP;       /*  Pointer to Code-page ID. */

APIRET     return;      /*  Return code. */



return = KbdGetCp(ulReserved, pidCP, hkbd);
