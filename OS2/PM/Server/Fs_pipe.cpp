/*
 $Id: Fs_pipe.cpp,v 1.4 2002/10/23 14:29:52 evgen2 Exp $
*/
/* Fs_pipe.cpp */
/* ver 0.00 20.08.2002       */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <builtin.h>
#include <time.h>
#include <process.h>

#include "FreePM.hpp"


#define INCL_DOSSEMAPHORES
#define INCL_DOSNMPIPES
#define INCL_DOSPROCESS
#include <os2.h>

#include "F_pipe.hpp"

void   sgLogError(char *, ...);

#define VERSION "0.12"

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
int InitQueue(void);
int ReadQueue(int threadNum);
int AddToQueue(int threadNum);

/*+---------------------------------+*/
/*| Internal function prototypes.   |*/
/*+---------------------------------+*/
int threadsHits_statistics(void);
void /*_Optlink*/ SQDR_ClientWork(void *param);

int sgCheckForReloadConfig(char *fname,int checktime);

int HandleClientQuery(char *buf, char *bufout, int nclient);
int QueryProcessType(void);

/*+--------------------------------------------------------+*/
/*| Static global variables and local constants.           |*/
/*+--------------------------------------------------------+*/

//class NPipe SQDRpipe;
class NPipe SQDR_pipe[FREEPMS_MAX_NUM_THREADS];

struct SQDR_threads
{
   volatile int semaphore;    // семафор для блокировки одновременного доступа к остальным полям
   int num;          // число ниток
   int next_free;    // следующая свободная
   int working;      // =0 на старте, =1 после появления первого живого клиента (защелка)
   int Nclients;     // число живых клиентов
   int thread_id[FREEPMS_MAX_NUM_THREADS]; // id'ы ниток
   int hits_total[FREEPMS_MAX_NUM_THREADS]; // запросов всего по ниткам
   int hits_min  [FREEPMS_MAX_NUM_THREADS]; // запросов за минуту по ниткам
   int hits_hour [FREEPMS_MAX_NUM_THREADS]; // запросов за час    по ниткам
   int state [FREEPMS_MAX_NUM_THREADS];     // состояние нитки: 0-свободно, 1 - занято
};

struct SQDR_threads  SQDRthreads = { 0,0,0,0,0 };

volatile int semnumQueryInProgress=0; // семафор количества обрабатываемых запросов
volatile int numQueryInProgress=0;    // количество обрабатываемых запросов
volatile int semReconfigInProgress=0; // семафор реконфигурации
/* при выставленном semReconfigInProgress обработка запроса не производится (уходит в sleep),
   numQueryInProgress не увеличивается, чтение конфигурации начинается при numQueryInProgress=0
*/

char *globalLogDir = NULL;
char *progname;



int detachedMode=0;


int threadsHits_statistics(void)
{
/* Very non-precision timer */
static int t_sec=0;
static int t_min=0;
static int t_hour=0;
static int last_allhits=0, last_hits_min=0,last_hits_hour=0;
static int max_hits_hour=0, max_hits_min=0;
       int i,allhits, hits_hour,hits_min, activthreads;
       char str[400],str1[80];;

      if(++t_sec == 60)
      {
          if(++t_min == 60)
          {  ++t_hour;
               t_min = 0;
          }
          t_sec = 0;
      }

      if(t_min == 0 && t_hour == 0)
                               return 0;
      activthreads = 0;
      for(allhits=hits_hour=hits_min=i=0;i<SQDRthreads.num;i++)
      {       allhits   += SQDRthreads.hits_total[i];
              hits_hour += SQDRthreads.hits_hour[i];
              hits_min  += SQDRthreads.hits_min[i];
              activthreads += SQDRthreads.state[i];
      }

      if(last_allhits == allhits)
      {
        printf("%i, %i, %i %2i:%2i:%2i \r",(last_hits_min+hits_min)/2,last_hits_hour,allhits,
                                  t_hour,t_min,t_sec);
                fflush(stdout);
      }
      last_allhits = allhits;

      if(hits_hour >  max_hits_hour)
                 max_hits_hour = hits_hour;
      if(hits_min >  max_hits_min)
                 max_hits_min = hits_min;

     if(t_sec == 0) /* min begining */
     {
         for(i=0;i < SQDRthreads.num; i++) SQDRthreads.hits_min[i] = 0;
         last_hits_min = hits_min;

       if(t_min == 0) /* hour begining */
       {  sprintf(str,"Hits:%i, hits/hour:%i (",allhits,hits_hour);
          for(i=0;i < SQDRthreads.num; i++)
          {  sprintf(str1,"%i",SQDRthreads.hits_hour[i]);
             strcat(str,str1);
             if(i < SQDRthreads.num-1)
                           strcat(str,", ");
             SQDRthreads.hits_hour[i] = 0;
          }
          last_hits_hour= hits_hour;
          strcat(str,")");
          sprintf(str1,",Max/h=%i, Max/min=%i",max_hits_hour,max_hits_min);
          strcat(str,str1);
          sprintf(str1,"act.threads %i",activthreads);
          strcat(str,str1);
          //sgLogError("%s",str);
       }
     } /* endof if(t_sec == 0) */

      return 0;
}


/************************************/
void /*_Optlink*/ SQDR_ClientWork(void *param)
{   int i,rc, threadNum,id,idd;
    int ncmd,data,l;
    char str[512];
    char buf[MAX_PIPE_BUF], bufout[MAX_PIPE_BUF];

//   HAB   habThread4 = NULLHANDLE;           /* anchor block handle for thread */
//   HMQ   hmqThread4 = NULLHANDLE;           /* message queue handle           */
//   QMSG  qmsgThread4;                       /* message queue structure        */

//   habThread4 = WinInitialize( 0UL );
//   hmqThread4 = WinCreateMsgQueue( habThread4, 0UL );

    _control87(EM_UNDERFLOW,EM_UNDERFLOW);

    threadNum =  SQDRthreads.next_free;
    if(param)
           threadNum = * ((int *)param);
  if(!detachedMode)
    printf("Start thread %i \n",threadNum);
//    sgLogError("Start thread %i\n",threadNum);
    DosSleep(30); /* 300 */
/* */
//     if(threadNum) sprintf(str,"%s%i",SQDR_BASE_PIPE_NAME,threadNum);
//     else strcpy(str,SQDR_BASE_PIPE_NAME);

    SQDR_pipe[threadNum]  = NPipe(FREEPM_BASE_PIPE_NAME,SERVER_MODE,FREEPMS_MAX_NUM_THREADS,threadNum);
    rc = SQDR_pipe[threadNum].Create();
    if(rc)
    {  printf("Error pipe creating  %s rc=%i",FREEPM_BASE_PIPE_NAME,rc);
       if(rc == ERROR_INVALID_PARAMETER)
                   printf("(INVALID PARAMETER)");
       sgLogError("Error pipe creating %s rc=%i",FREEPM_BASE_PIPE_NAME,rc);
       exit(1);
    }

//    rc = SQDR_pipe[threadNum].Create();
//    if(rc)
//    {  printf("Error pipe creating  %s rc=%i",str,rc);
//       if(rc == ERROR_INVALID_PARAMETER)
//                   printf("(INVALID PARAMETER)");
//       sgLogError("Error pipe creating %s rc=%i",str,rc);
//
//       exit(1);
//
//    }
    rc = SQDR_pipe[threadNum].Connect();
    if(rc)
    {   sgLogError("Error connectint pipe rc=%i, exit",rc);
        exit(1);
    }
    rc = SQDR_pipe[threadNum].HandShake();
    if(rc)
    {   sgLogError("Error HandShake pipe rc=%i, exit",rc);
        exit(1);
    }

/***********/
   idd = SQDRthreads.next_free;
   id = _beginthread(SQDR_ClientWork,NULL, THREAD_STACK_SIZE,&idd);
   while(__lxchg(&SQDRthreads.semaphore, LOCKED)) DosSleep(1);
    SQDRthreads.Nclients++;     // число живых клиентов
    SQDRthreads.working = 1;    // =0 на старте, =1 после появления первого живого клиента (защелка)
   SQDRthreads.num++;
   SQDRthreads.thread_id[SQDRthreads.next_free] = id;
   if(SQDRthreads.thread_id[SQDRthreads.next_free+1] == -1) SQDRthreads.next_free++;
   else
   {  for(i=0;i<FREEPMS_MAX_NUM_THREADS;i++)
      {  if(SQDRthreads.thread_id[i] == -1) SQDRthreads.next_free = i;
      }
   }
   __lxchg(&SQDRthreads.semaphore, UNLOCKED);
    SQDRthreads.state[threadNum]=0;
    DosSetPriority( PRTYS_THREAD, PRTYC_REGULAR, +15L, 0UL );

/* команды              */
/* 0 - закончить        */
/* 1 - запрос клиента   */
/* 2 - прервать вав     */
   do
   {
      SQDRthreads.state[threadNum]=0;
      rc = SQDR_pipe[threadNum].RecvCmdFromClient(&ncmd,&data);
      if(rc)
      {
         if(rc == -1)
         {  rc = SQDR_pipe[threadNum].QueryState();
            if(rc == ERROR_BAD_PIPE || rc == ERROR_PIPE_NOT_CONNECTED)
                                  break; // клиент подох ??
         }
         if(!detachedMode)
                printf("Recv error=%i\n",rc);
          sgLogError("Recv error=%i, thread end",rc);
          goto   ENDOFTHREAD;
//          exit(1);
      }
      SQDRthreads.state[threadNum]=1;
//      printf("Cmd=%i,data=%i\n",ncmd,data);
//   { static int threadNumOld=-1;
//
//      printf("Cmd=%i threadNum,=%i\r",ncmd, threadNum);
//     if(threadNum != threadNumOld)  printf("\n");
//     threadNumOld = threadNum;
//
//   }

      switch(ncmd)
      {
         case 1: /* обычный запрос редиректора */
//               rc = SQDR_pipe[threadNum].RecvDataFromClient(buf,&l, sizeof(buf));
//               HandleClientQuery(buf,bufout,threadNum);
//               l = strlen(bufout) + 1;
//               rc=SQDR_pipe[threadNum].SendCmdToServer(1, l);
//               rc=SQDR_pipe[threadNum].SendDataToServer(bufout, l);

            break;
         case 2: /* запрос анализатора - классификация урла */
//             { int Classification, redircode, idata[2];
//               rc = SQDR_pipe[threadNum].RecvDataFromClient(buf,&l, sizeof(buf));
//               HandleAnalyzerQuery(buf,bufout,threadNum,Classification, redircode);
//
//               idata[0] = redircode;
//               idata[1] = Classification;
//               rc=SQDR_pipe[threadNum].SendCmdToServer(2, l);
//               rc=SQDR_pipe[threadNum].SendDataToServer(idata, sizeof(int) * 2);
//             }
            break;
         case 3: /* запрос анализатора - наименование id'а классификации */
//            {   char str[40];
//                str[0] = 0;
//                if(data>=0 && data < numClassification)
//                {   strncpy(str,sClassification[data],40);
//                }
//               rc=SQDR_pipe[threadNum].SendCmdToServer(3, l);
//               rc=SQDR_pipe[threadNum].SendDataToServer(str, sizeof(char) * 40);
//            }
            break;

      }

   } while(ncmd);

ENDOFTHREAD:
    SQDRthreads.state[threadNum]=0;
    SQDR_pipe[threadNum].DisConnect();

   DosSetPriority( PRTYS_THREAD, PRTYC_REGULAR, -15L, 0UL );

   while(__lxchg(&SQDRthreads.semaphore, LOCKED))
                                       DosSleep(1);
   SQDRthreads.num--;
   SQDRthreads.thread_id[threadNum] = -1;
   SQDRthreads.next_free = threadNum;
   SQDRthreads.Nclients--;     // число живых клиентов
   __lxchg(&SQDRthreads.semaphore, UNLOCKED);

    DosSleep(16+threadNum*2);

//   if ( hmqThread4 != NULLHANDLE )
//      WinDestroyMsgQueue( hmqThread4 );
//   if ( habThread4 != NULLHANDLE )
//      WinTerminate( habThread4 );

//  _endthread();
}


/**************************************/

/* обработка запроса анализатора */
int HandleAnalyzerQuery(char *buf, char *bufout, int nclient,int &Classification, int &redircode)
{
    return 0;

}
//DosCreateQueue(

int HandleClientQuery(char *buf, char *bufout, int nclient)
{
   char *redirect;
   struct Source *src;
   char redirBuff[MAX_PIPE_BUF];
   int rc =0,Classification, redircode;

    if(buf == NULL || bufout == NULL)
                              return -1;
    while(semReconfigInProgress)
                       DosSleep(1);

//    if( sgCheckForReloadConfig(configFile,CHECK_CONFIG_TIME) )
//    {
//       while(__lxchg(&semReconfigInProgress, LOCKED))
//                                           DosSleep(1); //взведем семафор
//        DosBeep(1000,30);  //бикнем
//        while(numQueryInProgress)
//        {     DosSleep(1); //подождем, пока все запросы не выполнятся
//           if(!detachedMode)
//            {     printf("Reconf1 %i",numQueryInProgress);
//                  fflush(stdout);
//            }
//        }
//        rdmain.sgReadConfig(configFile);
//       __lxchg(&semReconfigInProgress, UNLOCKED);
//    }


//   while(__lxchg(&semnumQueryInProgress, LOCKED)) DosSleep(1);
//   numQueryInProgress++;
//   __lxchg(&semnumQueryInProgress, UNLOCKED);


    return rc;

RET:
//   while(__lxchg(&semnumQueryInProgress, LOCKED)) DosSleep(1);
//   numQueryInProgress--;
//   __lxchg(&semnumQueryInProgress, UNLOCKED);
//
//   rdmain.CheckForSaveHour();

    return rc;
}



void   sgLogError(char * aa, ...)
{
}

int ThreadPipe::InitClientConnection(char *externMachineName) {
        return 0;
}
