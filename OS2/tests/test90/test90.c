#define INCL_DOSMEMMGR

#include <os2.h>

int main(VOID)
{

PVOID     pb;    /*  The base virtual address of the gettable shared memory object as assigned by DosAllocSharedMem. */

ULONG     flag;  /*  Access protection flags. */

APIRET    ulrc;  /*  Return Code. */



ulrc = DosGetSharedMem(pb, flag);

return 0;
}
