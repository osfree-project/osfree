 #define INCL_DOSMODULEMGR     /* Module Manager values */

 #define INCL_DOSERRORS        /* Error values */

 #include <os2.h>

 #include <stdio.h>



int main(VOID) {



 PSZ      ModuleName     = "C:\\OS2\\DLL\\DISPLAY.DLL";  /* Name of module   */

 UCHAR    LoadError[256] = "";          /* Area for Load failure information */

 HMODULE  ModuleHandle   = NULLHANDLE;  /* Module handle                     */

 PFN      ModuleAddr     = 0;           /* Pointer to a system function      */

 ULONG    ModuleType     = 0;           /* Module type                       */

 APIRET   rc             = NO_ERROR;    /* Return code                       */



   rc = DosLoadModule(LoadError,               /* Failure information buffer */

                      sizeof(LoadError),       /* Size of buffer             */

                      ModuleName,              /* Module to load             */

                      &ModuleHandle);          /* Module handle returned     */

   if (rc != NO_ERROR) {

      printf("DosLoadModule error: return code = %u\n", rc);

      return 1;

   } else {

      printf("Module %s loaded.\n", ModuleName);

   } /* endif */



   rc = DosQueryProcAddr(ModuleHandle,         /* Handle to module           */

                         1L,                   /* No ProcName specified      */

                         NULL,                 /* ProcName (not specified)   */

                         &ModuleAddr);         /* Address returned           */

   if (rc != NO_ERROR) {

      printf("DosQueryProcAddr error: return code = %u\n", rc);

      return 1;

   } else printf("Address of module is 0x%x.\n", ModuleAddr);



   rc = DosQueryProcType(ModuleHandle,         /* Handle to module           */

                         1L,                   /* Indicate no ProcName given */

                         NULL,                 /* ProcName (not specified)   */

                         &ModuleType);         /* Type 0=16-bit   1=32-bit   */

   if (rc != NO_ERROR) {

      printf("DosQueryProcType error: return code = %u\n", rc);

      return 1;

   } else printf("This is a %s module.\n", ( ModuleType ? "32-bit" : "16-bit"));



   rc = DosFreeModule(ModuleHandle);

   if (rc != NO_ERROR) {

      printf("DosFreeModule error: return code = %u\n", rc);

      return 1;

   } else printf("Module %s freed.\n", ModuleName);



   return NO_ERROR;

}
