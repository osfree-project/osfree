#define INCL_DOSPROCESS       /* Process and thread values */
#define INCL_DOS
#define INCL_DOSERRORS        /* DOS error values          */
#define INCL_VIO
#include <os2.h>;

#include <stdio.h>
#include <conio.h>

int exec(VOID) {
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

void showTasks(void)
{
  long i;
  char ch;
  long right=1;

  while (1)
  {

  VioSetCurPos(0, 0, 0);
  if (right)
  printf("旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커�袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴�");
  else
  printf("�袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴쁘컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");

  for (i=0;i<=20;i++)
  printf("%s                                      %s%s                                      %s", right ? "�" : "�", right ? "�" : "�", right ? "�" : "�", right ? "�" : "�");
  if (right)
  printf("읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸훤袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴暠\n");
  else
  printf("훤袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴袴暠읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸\n");

  ch=getch();

  if (ch=='\t') if (right) right=0; else right=1;
  if (ch==' ') return;

  };

}

void main(void)
{
//  readConfig();
  showTasks();
};
