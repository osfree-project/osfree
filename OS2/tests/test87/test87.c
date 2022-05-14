#define INCL_DOSMISC

#include <os2.h>

int main(VOID)
{

PCHAR     *pTable;     /*  Pointer table. */

ULONG      cTable;     /*  Number of variable insertion text strings. */

PCHAR      pBuf;       /*  The address of the caller's buffer area where the system returns the requested message. */

ULONG      cbBuf;      /*  The length, in bytes, of the caller's buffer area. */

ULONG      msgnumber;  /*  The message number requested. */

PSZ        pszFile;    /*  The drive designation, path, and name of the file where the message can be found. */

PULONG     pcbMsg;     /*  The actual length, in bytes, of the message returned. */

APIRET     ulrc;       /*  Return Code. */



ulrc = DosGetMessage(pTable, cTable, pBuf,

         cbBuf, msgnumber, pszFile, pcbMsg);

return 0;
}
