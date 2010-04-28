 #define INCL_DOSFILEMGR

 #define INCL_DOSERRORS

 #include <os2.h>

 #include <stdio.h>



 int main(VOID)

   {



    APIRET  rc = NO_ERROR;    /* Return code */



    rc = DosShutdown(0L);



    if (rc != NO_ERROR) {

        printf("DosShutdown error: return code = %u\n",rc);

        return 1;

    } else {

        printf("The file system has been prepared for shutdown.\n");

    }  /* endif */



    return NO_ERROR;

    }

