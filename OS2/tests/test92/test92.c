#define INCL_DOSMISC    /* Miscellaneous values */

#define INCL_DOSERRORS  /* DOS Error values */

#include <os2.h>

#include <stdio.h>

#include <string.h>



int main(VOID) {

UCHAR   *IvTable[3] = {0};                   /* Table of variables to insert */

UCHAR   MsgInput[30] = "Processing for %1: %2 has %3. ";

UCHAR   DataArea[80]= "";                    /* Output message area */

ULONG   MsgLength   = 0;                     /* Length of returned message */

APIRET  rc          = 0;                     /* Return code */

int     LoopCtr     = 0;                     /* for loop counter */



IvTable[0] = "function";

IvTable[1] = "DosInsertMessage";

IvTable[2] = "started";



               /* Insert strings in proper variable fields */



rc = DosInsertMessage(IvTable,             /* Message insert pointer array */

                      3,                   /* Number of inserts */

                      MsgInput,            /* Input message */

                      strlen(MsgInput),    /* Length of input message */

                      DataArea,            /* Output area for message */

                      sizeof(DataArea),    /* Size of output area */

                      &MsgLength);        /* Length of output message created */

if (rc != NO_ERROR) {

   printf("DosInsertMessage error: return code = %u\n", rc);

   return 1;

}



printf("%s\n", DataArea);   /* Print the resulting message */



return NO_ERROR;

}
