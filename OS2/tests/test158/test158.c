 #define INCL_DOSMVDM     /* Multiple DOS sessions values */

 #define INCL_DOSERRORS   /* DOS Error values */

 #include <os2.h>

 #include <stdio.h>



 UCHAR   VDDName[10] = "VDD007";    /* Name of VDD */

 HVDD    VDDHandle   = NULLHANDLE;  /* Handle of VDD */

 SGID    SessionID   = 0;           /* Session identifier (should be initialized */

 ULONG   Command     = 3;           /* VDD function code (hypothetical) */

 APIRET  rc          = NO_ERROR;    /* Return code */

 UCHAR   InputBuffer[30]  = "Your command here";  /* Command information    */

 UCHAR   OutputBuffer[30] = "";                   /* Output data (returned) */



    rc = DosOpenVDD(VDDName, &VDDHandle);

    if (rc != NO_ERROR) {

       printf("VDD %s was not found.\n", rc);

       return 1;

    }



    rc = DosRequestVDD(VDDHandle,              /* Handle of VDD */

                       SessionID,              /* Session ID */

                       Command,                /* Command to send to VDD */

                       sizeof(InputBuffer),    /* Length of input */

                       InputBuffer,            /* Input buffer */

                       sizeof(OutputBuffer),   /* Length of output area */

                       OutputBuffer);          /* Output from command */

    if (rc != NO_ERROR) {

        printf("DosRequestVDD error: return code = %u\n", rc);

        return 1;

    }



    rc = DosCloseVDD(VDDHandle);               /* Close the VDD */

    if (rc != NO_ERROR) {

        printf("DosCloseVDD error: return code = %u\n", rc);

        return 1;

    }
