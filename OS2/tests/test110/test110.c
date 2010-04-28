 #define INCL_DOSFILEMGR   /* File Manager values */

 #define INCL_DOSERRORS    /* DOS error values    */

 #include <os2.h>

 #include <stdio.h>



int main(VOID) {

   UCHAR    EnumBuf[200] = {0};      /* Data Buffer */

   ULONG    ulEnumCnt    = 0;        /* Count of entries to return */

   FEA2     *ptr         = NULL;     /* Pointer to data items returned */

   ULONG    ulTemp       = 0;

   APIRET   rc           = NO_ERROR; /* Return code  */

   ULONG    i            = 0;        /* Loop index */

   FHLOCK FileHandleLock = 0;        /* File handle lock   */



   ulEnumCnt = (ULONG)-1; /* Request as many attributes as will fit in buffer */



   rc = DosProtectEnumAttribute(ENUMEA_REFTYPE_PATH,   /* ASCIIZ string       */

                         "c:\\os2\\attrib.exe", /* Name of file               */

                         1L,                    /* Start with first attribute */

                         (PVOID)&EnumBuf,       /* Buffer for information     */

                         sizeof(EnumBuf),

                         &ulEnumCnt,

                         ENUMEA_LEVEL_NO_VALUE,  /* Request level 1 info      */

                         FileHandleLock);       /* File handle lock           */

   if (rc != NO_ERROR) {

      printf("DosProtectEnumAttribute error: return code = %u\n", rc);

      return 1;

   }



   ptr = (FEA2 *)EnumBuf; /* Mask the buffer pointer to an FEA2 structure */



   printf ("Attributes found = %u\n", ulEnumCnt);



   for (i = 0; i < ulEnumCnt; i++) {

      printf ("name = %s\n", ptr->szName);       /* increment the ptr */

      ulTemp = ptr->oNextEntryOffset + (ULONG)ptr; /* with the value in */

      ptr = (FEA2 *)ulTemp;                      /* oNextEntryOffset  */

   } /* endfor */                                /* to access next record */



   return NO_ERROR;

}
