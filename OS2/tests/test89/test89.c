#define INCL_DOSMODULEMGR

#include <os2.h>



HMODULE    hmod;    /*  Handle of module that has the required resource. */

ULONG      idType;  /*  The type identifier of the 32-bit resource. */

ULONG      idName;  /*  The name identifier of the 32-bit resource. */

PPVOID     ppb;     /*  A pointer to the resource. */

APIRET     ulrc;    /*  Return Code. */



ulrc = DosGetResource(hmod, idType, idName,

         ppb);
