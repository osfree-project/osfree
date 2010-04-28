 #define INCL_DOSPROCESS       /* Process and thread values */

 #define INCL_DOS

 #define INCL_DOSERRORS        /* DOS error values          */

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {

 UCHAR       LoadError[CCHMAXPATH] = {0};

 PSZ         Args             = NULL;

 PSZ         Envs             = NULL;

 RESULTCODES ChildRC          = {0};

 APIRET      rc               = NO_ERROR;  /* Return code */



   rc = DosExecPgm(LoadError,           /* Object name buffer           */

                   sizeof(LoadError),   /* Length of object name buffer */

                   EXEC_SYNC,           /* Asynchronous/Trace flags     */

                   Args,                /* Argument string              */

                   Envs,                /* Environment string           */

                   &ChildRC,            /* Termination codes            */

                   "qfsinfo.exe");      /* Program file name            */



   if (rc != NO_ERROR) {

      printf("DosExecPgm error: return code = %u\n",rc);

      return 1;

   } else {

      printf("DosExecPgm complete.  Termination Code: %u  Return Code: %u\n",

              ChildRC.codeTerminate,

              ChildRC.codeResult);  /* This is explicitly set by other pgm */

   } /* endif */



   return NO_ERROR;

   }


 #define INCL_DOSPROCESS       /* Process and thread values */

 #define INCL_DOSERRORS        /* DOS error values          */

 #define INCL_DOS

 #include <os2.h>

 #include <stdio.h>



 int main(VOID) {

 UCHAR       uchLoadError[CCHMAXPATH] = {0}; /* Error info from DosExecPgm */

 RESULTCODES ChildRC           = {0};       /* Results from child process  */

 PID         pidChild          = 0;         /* pid for child process       */

 APIRET      rc                = NO_ERROR;  /* Return code                 */

 UCHAR       uchEnvString[14]  = "ANYTHING=HERE\0";  /* Environment string */

 UCHAR       uchArgString[14]  = "CHKDSK\0 C: \0";   /* Argument string    */



       /* The argument string consists of the following:

             - the name of the program (followed by a NULL)

             - any parameters supplied to the program (followed by 2 NULLs)

          Only 1 NULL is explicitly specified at the end of this string.

          ASCIIZ strings end with a NULL already, giving us 2 NULLs.       */



   rc = DosExecPgm(uchLoadError,           /* Object name buffer           */

                   sizeof(uchLoadError),   /* Length of object name buffer */

                   EXEC_ASYNCRESULT,       /* Asynchronous/Trace flags     */

                   uchArgString,           /* Argument string              */

                   uchEnvString,           /* Environment string           */

                   &ChildRC,               /* Returns pid of process on an

                                              asynchronous request         */

                   "CHKDSK.COM");          /* Program file name            */



   if (rc != NO_ERROR) {

      printf("DosExecPgm error: return code = %u\n",rc);

      return 1;

   }



   rc = DosWaitChild(DCWA_PROCESS,    /* Look at only the process specified */

                     DCWW_WAIT,       /* Wait until a child terminates      */

                     &ChildRC,        /* Termination codes returned         */

                     &pidChild,       /* pid of terminating process         */

                     ChildRC.codeTerminate);   /* Process (pid) to look at  */



   if (rc != NO_ERROR) {

      printf("DosWaitChild error: return code = %u\n",rc);

      return 1;

   } else {

      printf("Child complete.  Termination Code: %u  Return Code: %u\n",

              ChildRC.codeTerminate, ChildRC.codeResult);

   }

   return NO_ERROR;

  }
