 #define INCL_DOSPROCESS              /* Process and thread values */

 #define INCL_DOSERRORS               /* DOS error values          */

 #include <os2.h>

 #include <stdio.h>



 VOID APIENTRY ExitRtn1(VOID);        /* Our exit routine */



 int main(VOID) {



    APIRET rc = NO_ERROR;



    rc = DosExitList(EXLST_ADD             /* Add to exit-list sequence     */

                     | 0x00002A00,         /* Invocation order is 42 (0x2A) */

                     (PFNEXITLIST) ExitRtn1); /* Specify added exit routine */



    if (rc != NO_ERROR) {

      printf("DosExitList error: return code = %u\n", rc);

      return 1;

    }



    printf("Routine main ends...\n");



    return NO_ERROR;

   }



      /* All exit list routines must be declared as VOID APIENTRY.

         This ensures the integrity of the stack.                  */



 VOID APIENTRY ExitRtn1(VOID)

    {

    APIRET  erc = NO_ERROR;                     /* Return code */



    printf("... but ExitRtn1 runs last.\n");



           /* Might want to save data or close files here */



    erc = DosExitList(EXLST_EXIT,               /* Exit         */

                      (PFNEXITLIST) NULL);



    if (erc != NO_ERROR) {

      printf("DosExitList error: return code = %u\n", erc);

    }



    return;

    }
