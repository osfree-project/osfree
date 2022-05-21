#define INCL_KBD

#include <os2.h>

int main(VOID)
{
APIRET    rc;       /*  Return code. */



rc = KbdGetLayout(pszName, hkbd);
return 0;
}
