#define INCL_DOSPROCESS

#define INCL_DOSSESMGR

#define INCL_DOSERRORS

#include <stdio.h>

#include <os2.h>



int main(VOID) {

   STARTDATA SData   = {0};

   PSZ       PgmTitle    = "CHKDSK for Current Drive",  /* Title      */

             PgmName     = "CHKDSK.COM";  /* Want to run CHKDSK       */

   APIRET    rc          = NO_ERROR;   /* Return code                 */

   PID       pidChild    = 0;          /* PID returned                */

   ULONG     ulSessID    = 0;          /* Session ID returned         */

   UCHAR     achObjBuf[100] = {0};     /* Buffer for error info       */



   SData.Length  = sizeof(STARTDATA);

   SData.Related = SSF_RELATED_CHILD;       /* Start a child session        */

   SData.FgBg    = SSF_FGBG_FORE;           /* Start session in foreground  */

   SData.TraceOpt = SSF_TRACEOPT_NONE;      /* No trace                     */



   SData.PgmTitle = PgmTitle;

   SData.PgmName = PgmName;

   SData.PgmInputs = ""; /* No parameters                                   */



   SData.InheritOpt = SSF_INHERTOPT_SHELL;     /* Inherit shell's environ.  */

   SData.SessionType = SSF_TYPE_WINDOWABLEVIO; /* Windowed VIO session      */

   SData.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_NOAUTOCLOSE;

   SData.InitXPos  = 30;     /* Initial window coordinates                  */

   SData.InitYPos  = 40;

   SData.InitXSize = 200;    /* Initial window size                         */

   SData.InitYSize = 140;

   SData.ObjectBuffer  = achObjBuf; /* Contains info if DosExecPgm fails    */

   SData.ObjectBuffLen = (ULONG) sizeof(achObjBuf);



   rc = DosStartSession(&SData, &ulSessID, &pidChild);  /* Start CHKDSK     */

   if (rc != NO_ERROR) {

      printf ("DosStartSession error : return code = %u\n", rc);

      return 1;

   } else {

      printf ("Child process has SessID of %u and PID of %u.\n", ulSessID,pidChild);

   }



   printf ("Waiting 30 seconds before terminating...\n");



   rc = DosSleep(30000L);         /* Wait 30 seconds for child to complete */

   if (rc != NO_ERROR) {

      printf ("DosSleep error : return code = %u\n", rc);

      return 1;

   }



   return NO_ERROR;

  }
