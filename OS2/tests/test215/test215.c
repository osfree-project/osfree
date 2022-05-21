#define INCL_KBD

#include <os2.h>

int main(VOID)
{
PUSHORT    pidCP;       /*  Pointer to Code-page ID. */
APIRET     rc;      /*  Return code. */

rc = KbdGetCp(ulReserved, pidCP, hkbd);

return 0;
}
