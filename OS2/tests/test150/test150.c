#define INCL_DOSRESOURCES     /* Resource types */

#define INCL_DOSMODULEMGR     /* Module Manager values */

#define INCL_DOSERRORS        /* DOS error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

UCHAR     LoadError[256] = "";            /* Area for Load failure information */

PSZ       ModuleName = "C:\\OS2\\DLL\\PMWP.DLL";  /* DLL with resources */

HMODULE   ModHandle  = NULLHANDLE;        /* Handle for module */

ULONG     Size       = 0;                 /* Resource size */

APIRET    rc         = NO_ERROR;          /* API return code */



 rc = DosLoadModule(LoadError,               /* Failure information buffer */

                    sizeof(LoadError),       /* Size of buffer             */

                    ModuleName,              /* Module to load             */

                    &ModHandle);             /* Module handle returned     */

 if (rc != NO_ERROR) {

    printf("DosLoadModule error: return code = %u\n", rc);

    return 1;

 }



 rc = DosQueryResourceSize(ModHandle,     /* Handle for DLL containing resources */

                           RT_POINTER,    /* Ask for  Pointer                    */

                           1L,            /*          with an ID of 1            */

                           &Size);        /* The resource size is returned.      */



 if (rc != NO_ERROR) {

    printf("DosGetResource error: return code = %u\n", rc);

    return 1;

 } else {

    printf("Resource is %u bytes in size.\n", Size);

 } /* endif */



return NO_ERROR;

}
