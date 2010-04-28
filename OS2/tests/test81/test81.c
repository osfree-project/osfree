#define INCL_DOSRESOURCES     /* Resource types */

#define INCL_DOSMODULEMGR     /* Module Manager values */

#define INCL_DOSERRORS        /* DOS error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

UCHAR     LoadError[256] = "";         /* Area for Load failure information */

PSZ       ModuleName = "C:\\OS2\\DLL\\PMWP.DLL";  /* DLL with resources */

HMODULE   ModHandle  = NULLHANDLE;        /* Handle for module */

PVOID     Offset     = NULL;              /* Pointer to resource */

APIRET    rc         = NO_ERROR;          /* API return code */



 rc = DosLoadModule(LoadError,               /* Failure information buffer */

                    sizeof(LoadError),       /* Size of buffer             */

                    ModuleName,              /* Module to load             */

                    &ModHandle);             /* Module handle returned     */

 if (rc != NO_ERROR) {

    printf("DosLoadModule error: return code = %u\n", rc);

    return 1;

 }



 rc = DosGetResource(ModHandle,     /* Handle for DLL containing resources */

                     RT_POINTER,    /* Ask for  Pointer                    */

                     1L,            /*          with an ID of 1            */

                     &Offset);      /* Get back pointer                    */

 if (rc != NO_ERROR) {

    printf("DosGetResource error: return code = %u\n", rc);

    return 1;

 } else {

    printf("Resource Offset = 0x%x\n", Offset);

 } /* endif */



 rc = DosFreeResource(Offset);

 if (rc != NO_ERROR) {

    printf("DosFreeResource error: return code = %u\n", rc);

    return 1;

 }



return NO_ERROR;

}
