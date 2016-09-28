#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <malloc.h>

int format(int argc, char *argv[], char *envp[]);
int chkdsk(int argc, char *argv[], char *envp[]);
int recover(int argc, char *argv[], char *envp[]);
int sys(int argc, char *argv[], char *envp[]);

short _Far16 _Pascal _loadds CHKDSK(short argc, char *_Seg16 *_Seg16 argv, char *_Seg16 *_Seg16 envp)
{
   short Argc = argc;
   char  **Argv = NULL;
   char  **Envp = NULL;
   char  * _Seg16 * _Seg16 p;
   short rc;
   int   i, n;

   Argv = (char **)malloc((Argc + 1) * sizeof(char * _Seg16));

   for (i = 0, p = argv; i < argc; i++, p++)
       Argv[i] = (char *)(*p);

   Argv[argc] = NULL;

   for (n = 0, p = envp; p && *p; n++, p++) ;

   Envp = (char **)malloc((n + 1) * sizeof(char * _Seg16));

   for (i = 0, p = envp; i < n; i++, p++)
       Envp[i] = (char *)(*p);

   Envp[n] = NULL;
   
   rc = chkdsk(Argc, Argv, Envp);

   free(Envp); free(Argv);
   return rc;
}

short _Far16 _Pascal _loadds FORMAT(short argc, char *_Seg16 *_Seg16 argv, char *_Seg16 *_Seg16 envp)
{
   short Argc = argc;
   char  **Argv;
   char  **Envp;
   char  * _Seg16 * _Seg16 p;
   short rc;
   int   i, n;

   Argv = (char **)malloc(Argc * sizeof(char * _Seg16));

   for (i = 0, p = argv; i < argc; i++, p++)
       Argv[i] = (char *)(*p);

   for (n = 0, p = envp; p && *p; n++, p++) ;

   Envp = (char **)malloc(n * sizeof(char * _Seg16));

   for (i = 0, p = envp; i < n; i++, p++)
       Envp[i] = (char *)(*p);
   
   rc = format(Argc, Argv, Envp);

   free(Envp); free(Argv);

   return rc;
}

short _Far16 _Pascal _loadds RECOVER(short argc, char *_Seg16 *_Seg16 argv, char *_Seg16 *_Seg16 envp)
{
   short Argc = argc;
   char  **Argv;
   char  **Envp;
   char  * _Seg16 * _Seg16 p;
   short rc;
   int   i, n;

   Argv = (char **)malloc(Argc * sizeof(char * _Seg16));

   for (i = 0, p = argv; i < argc; i++, p++)
       Argv[i] = (char *)(*p);

   for (n = 0, p = envp; p && *p; n++, p++) ;

   Envp = (char **)malloc(n * sizeof(char * _Seg16));

   for (i = 0, p = envp; i < n; i++, p++)
       Envp[i] = (char *)(*p);
   
   rc = recover(Argc, Argv, Envp);

   free(Envp); free(Argv);

   return NO_ERROR;
}

short _Far16 _Pascal _loadds SYS(short argc, char *_Seg16 *_Seg16 argv, char *_Seg16 *_Seg16 envp)
{
   short Argc = argc;
   char  **Argv;
   char  **Envp;
   char  * _Seg16 * _Seg16 p;
   short rc;
   int   i, n;

   Argv = (char **)malloc(Argc * sizeof(char * _Seg16));

   for (i = 0, p = argv; i < argc; i++, p++)
       Argv[i] = (char *)(*p);

   for (n = 0, p = envp; p && *p; n++, p++) ;

   Envp = (char **)malloc(n * sizeof(char * _Seg16));

   for (i = 0, p = envp; i < n; i++, p++)
       Envp[i] = (char *)(*p);
   
   rc = sys(Argc, Argv, Envp);

   free(Envp); free(Argv);

   return NO_ERROR;
}
