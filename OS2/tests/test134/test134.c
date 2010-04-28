#define INCL_DOSFILEMGR   /* File manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #include <os2.h>

 #include <stdio.h>

 #include <string.h>



int main(VOID)

  {

  UCHAR  szDeviceName[8] = "C:";  /* Device name or drive letter string */

  ULONG  ulOrdinal       = 0;     /* Ordinal of entry in name list      */

  PBYTE  pszFSDName      = NULL;  /* pointer to FS name                 */

  PBYTE  prgFSAData      = NULL;  /* pointer to FS data                 */

  APIRET rc              = NO_ERROR; /* Return code                     */



    /* Return-data buffer should be large enough to hold FSQBUFFER2 */

    /* and the maximum data for szName, szFSDName, and rgFSAData    */

    /* Typically, the data isn't that large.                        */



  BYTE         fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};

  ULONG        cbBuffer   = sizeof(fsqBuffer);        /* Buffer length) */

  PFSQBUFFER2  pfsqBuffer = (PFSQBUFFER2) fsqBuffer;



    rc = DosQueryFSAttach(

                szDeviceName,    /* Logical drive of attached FS      */

                ulOrdinal,       /* ignored for FSAIL_QUERYNAME       */

                FSAIL_QUERYNAME, /* Return data for a Drive or Device */

                pfsqBuffer,      /* returned data                     */

                &cbBuffer);      /* returned data length              */



    /* On successful return, the fsqBuffer structure contains     */

    /* a set of information describing the specified attached     */

    /* file system and the DataBufferLen variable contains        */

    /* the size of information within the structure               */



    if (rc != NO_ERROR) {

       printf("DosQueryFSAttach error: return code = %u\n", rc);

       return 1;

    } else {



       /* The data for the last three fields in the FSQBUFFER2    */

       /* structure are stored at the offset of fsqBuffer.szName. */

       /* Each data field following fsqBuffer.szName begins       */

       /* immediately after the previous item.                    */



       pszFSDName = pfsqBuffer->szName + pfsqBuffer->cbName + 1;

       prgFSAData = pszFSDName + pfsqBuffer->cbFSDName + 1;



       printf("iType     = %d\n", pfsqBuffer->iType);

       printf("szName    = %s\n", pfsqBuffer->szName);

       printf("szFSDName = %s\n", pszFSDName);

       printf("rgFSAData = %s\n", prgFSAData);

    }

   return NO_ERROR;

}
