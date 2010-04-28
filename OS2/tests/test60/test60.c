 #define INCL_DOSDEVICES   /* Device values */

 #define INCL_DOSERRORS    /* Error values */

 #include <os2.h>

 #include <stdio.h>



int main(VOID) {



 BYTE    Model      = 0,  /* Model number */

         SubModel   = 0,  /* Submodel */

         Adapter    = 0,  /* Display adapter type */

         CoProc     = 0,  /* Math coprocessor installed? */

         Printers   = 0;  /* Number of printers */



 APIRET  rc         = NO_ERROR; /* Return code */



    rc = DosDevConfig(&Model, DEVINFO_MODEL);

    if (rc != NO_ERROR) {

       printf("DosDevConfig error:  return code = %u\n", rc);

       return 1;

    }



    rc = DosDevConfig(&SubModel, DEVINFO_SUBMODEL);

    printf("    Model/SubModel:  %d / %d\n", Model, SubModel);



    rc = DosDevConfig(&Adapter, DEVINFO_ADAPTER);

    printf("      Display type:  %s\n", (Adapter ? "Color" : "Monochrome"));



    rc = DosDevConfig(&CoProc, DEVINFO_COPROCESSOR);

    printf(" Math co-processor?  %s\n", (CoProc ? "Yes" : "No"));



    rc = DosDevConfig(&Printers, DEVINFO_PRINTER);

    printf("Number of printers:  %d\n", Printers);



    return NO_ERROR;

}
