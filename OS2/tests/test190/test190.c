#define INCL_DOSSESMGR

#define INCL_DOSERRORS

#include <stdio.h>

#include <os2.h>



int main(VOID) {

   STARTDATA SData       = {0};

   PSZ       PgmTitle    = "Test Program", /* Title of new session     */

             PgmName     = "CMD.EXE";  /* This starts an OS/2 session  */

   APIRET    rc          = NO_ERROR;   /* Return code                  */

   PID       pid         = 0;          /* PID returned                 */

   ULONG     ulSessID    = 0;          /* Session ID returned          */

   UCHAR     achObjBuf[256] = {0};     /* Error data if DosStart fails */



   SData.Length  = sizeof(STARTDATA);

   SData.Related = SSF_RELATED_INDEPENDENT; /* start an independent session */

   SData.FgBg    = SSF_FGBG_FORE;           /* start session in foreground  */

   SData.TraceOpt = SSF_TRACEOPT_NONE;      /* No trace                     */

             /* Start an OS/2 session using "CMD.EXE /K" */

   SData.PgmTitle = PgmTitle;

   SData.PgmName = PgmName;

   SData.PgmInputs = "/K";                     /* Keep session up           */



   SData.TermQ = 0;                            /* No termination queue      */

   SData.Environment = 0;                      /* No environment string     */

   SData.InheritOpt = SSF_INHERTOPT_SHELL;     /* Inherit shell's environ.  */

   SData.SessionType = SSF_TYPE_WINDOWABLEVIO; /* Windowed VIO session      */

   SData.IconFile = 0;                         /* No icon association       */

   SData.PgmHandle = 0;

           /* Open the session VISIBLE and MAXIMIZED */

   SData.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_MAXIMIZE;

   SData.InitXPos  = 30;     /* Initial window coordinates              */

   SData.InitYPos  = 40;

   SData.InitXSize = 200;    /* Initial window size */

   SData.InitYSize = 140;

   SData.Reserved = 0;

   SData.ObjectBuffer  = achObjBuf; /* Contains info if DosExecPgm fails */

   SData.ObjectBuffLen = (ULONG) sizeof(achObjBuf);



   rc = DosStartSession(&SData, &ulSessID, &pid);  /* Start the session */



   if (rc != NO_ERROR) {

      printf ("DosStartSession error : return code = %u\n", rc);

      return 1;

   }

   return NO_ERROR;

}
