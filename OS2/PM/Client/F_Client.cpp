/*
 $Id: F_Client.cpp,v 1.4 2002/11/21 13:19:04 evgen2 Exp $
*/
/* F_Client.cpp*/
#define POKA 0
#define DEBUG 0
/* ver 0.01 24.08.2002       */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "FreePM.hpp"

#define F_INCL_DOSSEMAPHORES
#define F_INCL_DOSNMPIPES
#define F_INCL_DOSPROCESS
   #include "F_OS2.hpp"
#include "F_pipe.hpp"

#include "F_win.hpp"


/*+---------------------------------+*/
/*| Global variables                |*/
/*+---------------------------------+*/
#include "F_globals.hpp"


/* Internal functions prototypes */

int SendQueryToServer(char *qurl, char *bufout,int bufoutlen);
int QueryProcessType(void);
//int QueryThreadOrdinal(int &tid);

int CloseThreadPipe(void);


struct LogFileStat *globalErrorLog = NULL;
struct LogFile *globalLogFile = NULL;

struct LogFileStat *lastLogFileStat;
struct LogFileStat *LogFileStat;


char **globalArgv ;
char **globalEnvp ;

int globalDebug = 0;
int globalPid = 0;
char *globalLogDir = NULL;

int sockSquid= -1;
static const char *hello_string = "hi there\n";
char *progname;
char *ExternMachine = NULL;

#define LOCKED    1
#define UNLOCKED  0

//class NPipe SQDRpipe;
class ProccessPipes ProcPipes;
volatile int ProccessPipes::Access=UNLOCKED;


int    detachedMode=0;

HMTX   SQDR_hmtx     = NULLHANDLE; /* Mutex semaphore handle */
void  /*_Optlink*/ CloseAllClientConnection(void)
{   int i;
    for(i=0;i<32;i++)
    {  if(ProcPipes.pipe[i].used)
       {  if(ProcPipes.pipe[i].Hpipe)
           {  DosClose(ProcPipes.pipe[i].Hpipe);
              ProcPipes.pipe[i].Hpipe = NULL;
              ProcPipes.pipe[i].used = 0;
              ProcPipes.pipe[i].threadOrdinal = 0;
              ProcPipes.pipe[i].threadOrdinal = 0;
           }
       }
    }
}

int InitClientConnection(char *externMachineName)
{   int rc,ierr;
    ProcPipes.externMachineName = externMachineName;
    ProcPipes.ThreadPipeOpen(ierr);
    atexit(CloseAllClientConnection);
    return ierr;
}

int CloseThreadPipe(void)
{  int np;
   ProcPipes.CloseThread();
/*
   np = ProcPipes.QueryThreadPipeNum();
   if(np >= 0)
   {   if(ProcPipes.pipe[np].used && ProcPipes.pipe[np].Hpipe)
       {    DosClose(ProcPipes.pipe[np].Hpipe);
              ProcPipes.pipe[np].Hpipe = NULL;
              ProcPipes.pipe[np].used = 0;
              ProcPipes.pipe[np].threadOrdinal = 0;
        }
   }
*/
   return 0;
}

/*
   rc =
0 - Ok
1 - ERROR_INVALID_PARAMETER  - Hедопустимый паpаметp
2 - Can't open pipe(s)
3 - Error handshake  pipe, Exitting

*/

int ThreadPipe::InitClientConnection(char *externMachineName)
{
  int ch,i,ii,rep;
  struct Source *src;
static  char lastquery[MAX_PIPE_BUF]="",bufout[MAX_PIPE_BUF];
  char str[256];
  char *redirect;
  time_t t;
      int fd=-1;
      int rc, t0,t01;

  if(externMachineName && externMachineName[0]) ExternMachine = externMachineName;


/*****************************************************/

    rc = DosOpenMutexSem(FREEPM_MUTEX_NAME,      /* Semaphore name */
                         &SQDR_hmtx);            /* Handle returned */

//printf("[%i] DosOpenMutexSem rc=%i  ",globalPid,rc); fflush(stdout);

    DosCloseMutexSem(SQDR_hmtx);

/* запускаем сервер на локальной машине - если он уже запущен - сам вывалится */
   if(rc && !ExternMachine)
   {
      if(detachedMode)
      {  sprintf(str,"detach %s",_FreePM_Application_Name);
         rc = system(str);
         debug(1, 0)("%s, rc=%i",str,rc);
         if(rc == -1)
         {  debug(1, 0)("errno =%i, _doserrno=%i",errno,_doserrno);
         }
      } else {
//         printf("[%i]start /N sqdrserver ",globalPid); fflush(stdout);
         sprintf(str,"start /N %s",_FreePM_Application_Name);
         rc = system(str);
         printf("%s rc=%i\n",str,rc); fflush(stdout);

         if(rc == -1)
         {
           debug(1, 0)("errno =%i, _doserrno=%i",errno,_doserrno);
         }
     }
     DosSleep(200);
   }
   for(rep=0,i=0;i<MAX_NUM_PIPES*2,rep<40+MAX_NUM_PIPES;i++)
   {  ii = i % MAX_NUM_PIPES;
      if(ExternMachine)
      {  if(i) sprintf(buf,"\\\\%s\\%s%i",ExternMachine,FREEPM_BASE_PIPE_NAME,ii);
         else sprintf(buf,"\\\\%s\\%s",ExternMachine,FREEPM_BASE_PIPE_NAME);
      } else {
         if(i) sprintf(buf,"%s%i",FREEPM_BASE_PIPE_NAME,ii);
         else strcpy(buf,FREEPM_BASE_PIPE_NAME);
      }
//      SQDRpipe = NPipe(buf,CLIENT_MODE);
      strcpy(name,buf);
      rc = Open();
      if(rc == ERROR_PIPE_BUSY)
               continue;
      if(rc == ERROR_PATH_NOT_FOUND)
      {  rep++;
         DosSleep(50+rep);
         continue;
      }
      if(rc)
      {  printf("Error open pipe rc=%i",rc);
         debug(1, 0)("Error open pipe rc=%i, Exitting",rc);
         if(rc == ERROR_INVALID_PARAMETER) printf("(Hедопустимый паpаметp");
         return 1;
      }
      break;
    }
    if(!Hpipe)
    {  printf("Can't open pipe(s)");
       debug(1, 0)("Can't open pipe(s), Exitting");
       return 2;
    }
//    printf("Open pipe=%i\n",rc);
    rc = HandShake();
//    printf("HandShake=%i\n",rc);
    if(rc ==  HAND_SHAKE_ERROR)
    {
        printf("Error handshake\n",rc);
        debug(1, 0)("Error handshake  pipe, Exitting");
        DosBeep(2000,1);
        DosBeep(3000,1);
        return 3;
    }
    return 0;
}


/************************/
int SendQueryToServer(char *qurl, char *bufout,int bufoutlen)
{  int l,rc,ncmd,data,ierr;
  class  ThreadPipe *pPipe;
   if(!qurl)
      return -1;
   l = strlen(qurl);
   if(!l)
      return -2;
   pPipe = ProcPipes.ThreadPipeOpen(ierr);
   if(ierr)
      return ierr;
   l++; //+ zero at end of string
   rc = pPipe->SendCmdToServer(1,l); // команда 1 - послать данные о запросе
   if(rc)
      return rc;
   rc = pPipe->SendDataToServer((void *)qurl, l);
   if(rc)
      return rc;
   rc = pPipe->RecvCmdFromClient(&ncmd,&data);
   if(rc)
      return rc;
   switch(ncmd)
   {  case 1:
        l = data;
        rc = pPipe->RecvDataFromClient(bufout,&l,bufoutlen);
          break;

   }

   return rc;
}



