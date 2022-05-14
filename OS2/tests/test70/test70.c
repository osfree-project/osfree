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


