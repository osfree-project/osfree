#define INCL_DOSPROCESS   /* DOS process values - needed for DosSleep */

#define INCL_DOSSESMGR

#define INCL_DOSERRORS

#include <stdio.h>

#include <os2.h>



int main(VOID) {

   STARTDATA SData       = {0};

   PSZ       PgmTitle    = "Not in the Window List",    /* Title      */

             PgmName     = "CMD.EXE";  /* This starts an OS/2 session */

   APIRET    rc          = NO_ERROR;   /* Return code                 */

   PID       pid         = 0;          /* PID returned                */

   ULONG     ulSessID    = 0;          /* Session ID returned         */

   UCHAR     achObjBuf[100] = {0};     /* Error info if start fails   */

   STATUSDATA ChildStatus   = {0};     /* Child status data           */



   SData.Length  = sizeof(STARTDATA);

   SData.Related = SSF_RELATED_CHILD;       /* A dependent session          */

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

   printf("Removing child process from the Window List... \n");



   ChildStatus.Length = sizeof(STATUSDATA);

   ChildStatus.SelectInd = SET_SESSION_NON_SELECTABLE;

   ChildStatus.BondInd = SET_SESSION_UNCHANGED;



   rc = DosSetSession(ulSessID, &ChildStatus);

   if (rc != NO_ERROR) {

      printf ("DosSetSession error : return code = %u\n", rc);

      return 1;

   }



   printf("\nPress Ctrl-Esc  The child is no longer listed.\n");

   printf("\nProgram will terminate in 10 seconds...\n");

   rc = DosSleep(10000L);  /* wait 10 seconds */



   return NO_ERROR;

}

