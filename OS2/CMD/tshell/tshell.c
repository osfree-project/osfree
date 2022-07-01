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
  printf("ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟษออออออออออออออออออออออออออออออออออออออป");
  else
  printf("ษออออออออออออออออออออออออออออออออออออออปฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ");

  for (i=0;i<=20;i++)
  printf("%s                                      %s%s                                      %s", right ? "ณ" : "บ", right ? "ณ" : "บ", right ? "บ" : "ณ", right ? "บ" : "ณ");
  if (right)
  printf("ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤูศออออออออออออออออออออออออออออออออออออออผ\n");
  else
  printf("ศออออออออออออออออออออออออออออออออออออออผภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู\n");

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
