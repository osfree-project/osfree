#define INCL_DOSMODULEMGR   /* Module Manager values */

#define INCL_DOSERRORS      /* DOS Error values */

#include <os2.h>

#include <stdio.h>



int main(VOID) {

   PSZ     ModuleName    = "C:\\OS2\\DLL\\DISPLAY.DLL";  /* Module name   */

   HMODULE ModuleHandle  = NULLHANDLE;                   /* Module handle */

   APIRET  rc            = NO_ERROR;                     /* Return code   */



   rc = DosQueryModuleHandle(ModuleName,      /* Module to look for       */

                             &ModuleHandle);  /* Handle (returned)        */

   if (rc != NO_ERROR) {

      printf("DosQueryModuleHandle error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("Module handle = %u\n", ModuleHandle);

   }



   rc = DosQueryModuleName (ModuleHandle,   /* Module handle to query     */

                            256L,           /* Maximum length of result   */

                            ModuleName);    /* Module name returned       */



   if (rc != NO_ERROR) {

      printf("DosQueryModuleName error: return code = %u\n", rc);

      return 1;

   } else {

      printf ("Module name = %s\n", ModuleName);

   }

   return NO_ERROR;

}
