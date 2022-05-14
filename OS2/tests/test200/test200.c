
#define INCL_DOSPROCESS

#include <os2.h>

int main(VOID)
{
APIRET    rc;          /*  Return Code. */

rc = DosTmrQueryFreq(pulTmrFreq);

return 0;
}
