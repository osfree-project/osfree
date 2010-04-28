 #define INCL_DOSEXCEPTIONS   /* Exception values */

 #define INCL_DOSERRORS       /* Error values */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID)

   {

   ULONG   ulNestLevel = 0;  /* Global variable tracking nesting

                                of DosEnterMustComplete calls     */

   APIRET  rc = NO_ERROR;    /* Return code                       */



    rc = DosEnterMustComplete(&ulNestLevel);



    if (rc != NO_ERROR) {

        printf("DosEnterMustComplete error: return code = %u\n",rc);

        return 1;

    } else {

        printf("ulNestLevel = %u\n",ulNestLevel);

    }



        /* ADD BLOCK OF CODE THAT MUST COMPLETE HERE... */



    rc = DosExitMustComplete(&ulNestLevel);



    if (rc != NO_ERROR) {

        printf("DosExitMustComplete error: return code = %u\n",rc);

        return 1;

    } else {

        printf("ulNestLevel = %u\n",ulNestLevel);

    }



    return NO_ERROR;

    }
