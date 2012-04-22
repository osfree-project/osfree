/* Fc_main.cpp */
/* FreePM client side main*/
/* DEBUG: section 1     main client */
/* ver 0.01 22.08.2002       */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "FreePM.hpp"
#include "FreePM_err.hpp"
#include "Fc_config.hpp"
#include "F_def.hpp"

/*+---------------------------------+*/
/*| Global variables                |*/
/*+---------------------------------+*/
#include "F_globals.hpp"

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
#include <pmclient.h>

class F_ClientConfig FPM_config;

const char *const _FreePM_Application_Name = FREEPM_CLIENT_APPLICATION_NAME;
const char *const _FreePM_Application_Vers = FREEPM_VERSION;

extern "C" APIRET APIENTRY  _InitServerConnection(char *remotemachineName, ULONG *obj);
extern "C" APIRET APIENTRY  _CloseServerConnection(void);
extern "C" APIRET APIENTRY  _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size);
extern "C" APIRET APIENTRY  _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data);
extern "C" APIRET APIENTRY  _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen);
extern "C" APIRET APIENTRY  _F_SendDataToServer(void *recvobj, void *data, int len);
extern "C" APIRET APIENTRY  _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen);
extern "C" APIRET APIENTRY  _F_SendGenCmdToServer(void *recvobj, int cmd, int par);
extern "C" APIRET APIENTRY  _F_SendCmdToServer(void *recvobj, int ncmd, int data);
extern "C" void APIENTRY    _db_print(const char *format,...);

/*+---------------------------------+*/
/*| Static  variables               |*/
/*+---------------------------------+*/
static int nxDefault=800, nyDefault=600,  bytesPerPixelDefault=4;
static char *ExternMachine = NULL;   /* Name of external machine for pipes like \\MACHINE\PIPE\SQDR   */

/*+---------------------------------+*/
/*|     local constants.            |*/
/*+---------------------------------+*/
char PipeName[256];

//class FreePM_session session;
class NPipe *pF_pipe;
static volatile int AccessF_pipe_srv = UNLOCKED;
static volatile int AccessF_pipe_cli = UNLOCKED;

#define LOCK(sem)                               \
    {   int ilps_raz = 0, ilps_rc;              \
        do                                      \
        {  ilps_rc =  __lxchg(&sem,LOCKED);     \
           if(ilps_rc)                          \
           { if(++ilps_raz  < 3)  DosSleep(0);  \
             else            DosSleep(1);       \
           }                                    \
        } while(ilps_rc);          \
    }

#define UNLOCK(sem)  {__lxchg(&sem,UNLOCKED);}

/*+---------------------------------+*/
/*|     local variables.            |*/
/*+---------------------------------+*/
struct LS_threads
{
   volatile int semaphore;    // семафор для блокировки одновременного доступа к остальным полям
   int n;            // max thread index
   int num;          // число ниток
   int next_free;    // следующая свободная
   int working;      // =0 на старте, =1 после появления первого живого клиента (защелка)
   int Nclients;     // число живых клиентов
   int thread_id[FREEPMS_MAX_NUM_THREADS]; // id'ы ниток
/* tread state:
    -1 - not run,
    0  - wait connect,
    1  - wait handhake,
    2  - wait next thead start
    3  - wait command
    4  - execute command
    5  - closing
*/
   int state [FREEPMS_MAX_NUM_THREADS];
};

/*
void _srvThread(void *param)
{
  HMODULE  hmod;
  char     LoadErr[256];

  DosLoadModule(LoadErr, sizeof(LoadErr), "FPMSRV", &hmod);

  for (;;)
    DosSleep(100000);
}
 */

void ExecuteFreePMServer(void)
{
  char Buffer[CCHMAXPATH];
//  char srcdir[CCHMAXPATH];
  HMODULE  hmod;
  char     LoadErr[256];
  //int idd;
/*
  char CmdLineBuf[2048];
  char *CmdLine;
  char *P;
  RESULTCODES ResultCodes;
  char ApplierEXE[] = "FreePM.exe";

  CmdLine = CmdLineBuf;
  if((((ULONG)CmdLine+1024)&0xFFFF) < 1024)
      CmdLine += 1024;
  P = strcpy(CmdLine, ApplierEXE)+strlen(ApplierEXE)+1;
  *P = '"';
  P++;
  P = strcpy(P, Buffer)+strlen(Buffer);
  *P = '"';
  P++;
  *P = 0;
  P++;
  *P = 0;

  DosExecPgm(NULL, 0, EXEC_ASYNC, CmdLine, NULL, &ResultCodes, ApplierEXE);
 */
  DosLoadModule(LoadErr, sizeof(LoadErr), "FPMSRV", &hmod);
}


extern "C" APIRET APIENTRY _InitServerConnection(char *remotemachineName, ULONG *obj)
{
  int rc;
  char buf[256];

/* init time */

  //DosGetDateTime(&_FreePM_current_time);
  _FreePM_current_time=time(NULL);
  
  _FreePM_start = _FreePM_current_time;

/* First  let's look for FreePM.ini and read it if any */
  FPM_config.Read("FreePM.ini");

/* init debug */
  //_db_init(_FreePMconfig.Log.log, FPM_config.debugOptions);

  rc = QueryProcessType();
  if(rc == 4)
    _FreePM_detachedMode = 1;

  //if(_FreePM_detachedMode)
  //{
  //  debug(1, 0) ("Starting in detached mode %s version %s...\n",_FreePM_Application_Name, _FreePM_Application_Vers);
  //} else {
  //  debug(1, 0) ("Starting %s version %s...\n",_FreePM_Application_Name, _FreePM_Application_Vers);
  //}

   if(remotemachineName)
   {
     strcpy(FPM_config.ExternMachineName,remotemachineName);
     ExternMachine = &FPM_config.ExternMachineName[0];
   } else {
/* test for FreePM's semaphore  at local machine */
       //debug(1, 0)("Fc_main: FREEPM_MUTEX_NAME=%s \n", FREEPM_MUTEX_NAME );
       HMTX    FREEPM_hmtx     = NULLHANDLE; /* Mutex semaphore handle */
       rc = DosOpenMutexSem(FREEPM_MUTEX_NAME,    /* Semaphore name */
                            &FREEPM_hmtx);        /* Handle returned */
       DosCloseMutexSem(FREEPM_hmtx);
       if(rc)
       {  /* FreePM server is not running at local machine, let's look for FreePM.ini */
         //debug(1, 1)("FreePM server is not running at local machine, test for server at %s\n",FPM_config.ExternMachineName);
         ExternMachine = &FPM_config.ExternMachineName[0];
       } else {
         ExternMachine = NULL;
       }
   }

/* init connection to FreePM server */
   if(ExternMachine)
   {
      sprintf(buf,"\\\\%s\\%s",ExternMachine,FREEPM_BASE_PIPE_NAME);
   } else {
      strcpy(buf,FREEPM_BASE_PIPE_NAME);
   }
   strcpy(PipeName,buf);

   //debug(1, 0)("Pipe: FREEPM_BASE_PIPE_NAME=%s \n", FREEPM_BASE_PIPE_NAME);
   pF_pipe = new NPipe(PipeName,CLIENT_MODE);

   rc = pF_pipe->Open();
   if(rc)
   {
      //debug(1, 0)("F_pipe.Open rc = %i (%s)\n",rc,GetOS2ErrorMessage(rc));
      if(ExternMachine)
      {
        //debug(1, 0)("%s is not running at remote machine %s. Starting local server...\n",FREEPM_SERVER_APPLICATION_NAME,ExternMachine);
        ExecuteFreePMServer();

        DosSleep(1000);

        strcpy(PipeName, FREEPM_BASE_PIPE_NAME);

        delete pF_pipe;
        pF_pipe = new NPipe(PipeName,CLIENT_MODE);

        rc = pF_pipe->Open();
        //if(rc)
        //{
        //  debug(1, 0)("%s is not running at local machine, exitting...\n",FREEPM_SERVER_APPLICATION_NAME);
        //  _fatal_common("FreePM server not running");
        //}
      } //else {
      //    debug(1, 0)("%s is not running at local machine, exitting...\n",FREEPM_SERVER_APPLICATION_NAME);
      //    _fatal_common("FreePM server not running");
      //}
   }
    rc = pF_pipe->HandShake();
    if(rc ==  HAND_SHAKE_ERROR)
    {   //debug(1, 0)("Error handshake %i, pipe %s\n",rc,PipeName);
        //_fatal_common("Error handshake  pipe");
        return 1;
    }
//todo
    *obj = (APIRET)pF_pipe;

    rc = 0;
    return rc;
}


extern "C" APIRET APIENTRY  _CloseServerConnection(void)
{   

    if(pF_pipe)
    {
       debug(1, 0) ("CloseServerConnection\n");
       delete pF_pipe;
       pF_pipe = NULL;
    }

    return 0;
}


extern "C" APIRET APIENTRY  _F_SendCmdToServer(void *recvobj, int ncmd, int data)
{   int rc;
    LOCK(AccessF_pipe_cli);

    rc = ((class NPipe *)(recvobj))->SendCmdToServer(ncmd,  data);
    if(rc == ERROR_BROKEN_PIPE) /* attempt to reconnect till timeout */
    { int raz;
      debug(1, 1)(__FUNCTION__ " Broken Pipe, Try to reconnect...\n");
      for(raz = 0;raz < 10; raz++)
      {  delete ((class NPipe *)(recvobj));
         recvobj = NULL;
         DosSleep(100+raz*10);
         recvobj = new NPipe(PipeName,CLIENT_MODE);
         rc = ((class NPipe *)(recvobj))->Open();
         if(rc)
         {    debug(1, 1)(__FUNCTION__ " Broken Pipe, Server still down...\n");
         } else {
           rc = ((class NPipe *)(recvobj))->HandShake();
           if(rc ==  HAND_SHAKE_ERROR)
           {   debug(1, 0)("Error handshake %i, pipe %s\n",rc,PipeName);
           } else {
               break;
           }
         }
      }
    }

   UNLOCK(AccessF_pipe_cli);
   return rc;
}


/* Послать команду cmd c параметром par на сервер, получить ответ и вернуть его */
/* застрелиться при ошибках */
extern "C" APIRET APIENTRY  _F_SendGenCmdToServer(void *recvobj, int cmd, int par)
{   int rc,len,retcode;
    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if(rc)
    {  debug(1, 0)("WARNING:"__FUNCTION__":SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
       _fatal_common("SendCmdToServer Error\n");
    }
    rc = _F_RecvDataFromServer(recvobj, &retcode, &len, sizeof(int));
    if(rc)
    {  debug(1, 0)("WARNING:"__FUNCTION__":RecvDataFromServer Error: %s\n",GetOS2ErrorMessage(rc));
       _fatal_common("RecvDataFromServer Error\n");
    }
    debug(1, 3)( __FUNCTION__ "(cmd=%x, par=%x) ret =%x\n",cmd, par, retcode);

    return retcode;
}


/* Послать команду cmd c параметром par, данные data длиной datalen*sizeof(int) на сервер, получить ответ и вернуть его */
/* застрелиться при ошибках */
extern "C" APIRET APIENTRY  _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen)
{   int rc,len,retcode;
M:  rc = _F_SendCmdToServer(recvobj, cmd, par);
    if(rc)
    {  debug(1, 0)("WARNING:"__FUNCTION__":SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
       _fatal_common("SendCmdToServer Error\n");
    }
    rc =  _F_SendDataToServer(recvobj, data, datalen * sizeof(int));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {   debug(1, 1)("WARNING:"__FUNCTION__":SendDataToServer return Broken Pipe, try to reconnect...\n");
           goto M;/* attempt to reconnect till timeout */
       }
       debug(1, 0)("WARNING:"__FUNCTION__":SendDataToServer Error: %s\n",GetOS2ErrorMessage(rc));
       _fatal_common("SendCmdToServer Error\n");
    }

    rc = _F_RecvDataFromServer(recvobj, &retcode, &len, sizeof(int));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {   debug(1, 1)("WARNING:"__FUNCTION__":SendDataToServer return Broken Pipe, try to reconnect...\n");
           goto M;/* attempt to reconnect till timeout */
       }
       debug(1, 0)("WARNING:"__FUNCTION__":RecvDataFromServer Error: %s\n",GetOS2ErrorMessage(rc));
       _fatal_common("RecvDataFromServer Error\n");
    }
    debug(1, 3)( __FUNCTION__ " retcode =%x\n",retcode);

    return retcode;
}


extern "C" APIRET APIENTRY  _F_SendDataToServer(void *recvobj, void *data, int len)
{
   int rc;
   LOCK(AccessF_pipe_cli);

   rc = ((class NPipe *)(recvobj))->SendDataToServer(data,  len);
   debug(1, 9) ("Send data %i bytes, rc=%i\n", len, rc);

   UNLOCK(AccessF_pipe_cli);
   return rc;
}

extern "C" APIRET APIENTRY  _F_SendDataToClient(void *recvobj, void *data, int len)
{
   int rc;
   LOCK(AccessF_pipe_srv);

   rc = ((class NPipe *)(recvobj))->SendDataToServer(data,  len);
   debug(1, 9) ("Send data %i bytes, rc=%i\n", len, rc);

   UNLOCK(AccessF_pipe_srv);
   return rc;
}

extern "C" APIRET APIENTRY  _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen)
{
   int rc;
   LOCK(AccessF_pipe_cli);

   rc = ((class NPipe *)(recvobj))->RecvDataFromClient(data, len, maxlen);
   debug(1, 9) ("Recv data %i bytes, rc=%i\n", *len, rc);

   UNLOCK(AccessF_pipe_cli);
   return rc;
}


extern "C" APIRET APIENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data)
{
    int rc;
    LOCK(AccessF_pipe_srv);

    rc = ((class NPipe *)(recvobj))->RecvCmdFromClient(ncmd, data);

    UNLOCK(AccessF_pipe_srv);
    return rc;
}

extern "C" APIRET APIENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size)
{    int rc;
    LOCK(AccessF_pipe_srv);

     rc = ((class NPipe *)(recvobj))->RecvDataFromClient(sqmsg, l, size);

    UNLOCK(AccessF_pipe_srv);
    return rc;
}


struct LS_threads  LSthreads = { 0,0,0,0,0,0 };

class NPipe *FreePM_pipe[FREEPMS_MAX_NUM_THREADS];

int ThreadStart = 0;

void (*handler) (void *obj, int ncmd, int data, int threadNum);

void /*_Optlink*/  Fs_ClientWork(void *param)
{


    int i,rc, threadNum,id,idd;
    int ncmd,data;
//    char PipeName[256];
    char str[512];
    char buf[MAX_PIPE_BUF], bufout[MAX_PIPE_BUF];

    _control87(EM_UNDERFLOW,EM_UNDERFLOW);

   int bla = 0;
   while(__lxchg(&LSthreads.semaphore, LOCKED))
                                       DosSleep(1);

   //int volatile * a, int  b) {
        /*printf("FIXME: builtin.h:%s (%d, %d)\n", __func__, *a, b);*/
        //DosEnterCritSec();
        /*
        int ret = 1;
        while(ret) {
        ret = 1;
        if(LSthreads.semaphore == 0 && LOCKED == 1) {
                LSthreads.semaphore = LOCKED;
                ret = 0;
        }
        if(LSthreads.semaphore == 1 && LOCKED == 0) {
                ret = 1;
        }
        for(double di=0; di>100; di++)
                di++;
        }
        */
        //DosExitCritSec();
        //return ret;

    threadNum =  LSthreads.next_free;
    LSthreads.state[threadNum] = -1;
    if(LSthreads.n < threadNum)
              LSthreads.n = threadNum;
    debug(0, 2) ("Fs_ClientWork%i: Pipe creating  (%i)\n",threadNum,LSthreads.thread_id[threadNum]); // 2
    fflush(stdout);
    ThreadStart++;
   DosSleep(1);

   __lxchg(&LSthreads.semaphore, UNLOCKED);
   do
   { DosSleep(1);
   } while(ThreadStart != 3);

//    if(param)
//           threadNum = * ((int *)param);

    DosSleep(1);

    FreePM_pipe[threadNum]  = new NPipe(FREEPM_BASE_PIPE_NAME,SERVER_MODE,FREEPMS_MAX_NUM_THREADS,threadNum);

    rc = FreePM_pipe[threadNum]->Create();
    if(rc == ERROR_TOO_MANY_OPEN_FILES)
    {     rc = OS2SetRelMaxFH(8);
          rc = FreePM_pipe[threadNum]->Create();
    }

    if(rc)
    {  snprintf(str,256, "Error pipe creating  %s rc=%i",FreePM_pipe[threadNum]->name,rc);
       if(rc == ERROR_INVALID_PARAMETER)
                   strcat(str,"(INVALID PARAMETER)");
       else
          if (rc == ERROR_PIPE_BUSY)
                   strcat(str,"(PIPE_BUSY)");
       if (rc == ERROR_PIPE_BUSY)
       {    debug(0, 0) ("WARNING: Fs_ClientWork: %s\n",str);
            goto ENDTHREAD;
       }
       _fatal_common(str);
    }
    LSthreads.state[threadNum] = 0;
    debug(0, 2) ("Fs_ClientWork%i: Pipe create %s %x %x\n",threadNum,FreePM_pipe[threadNum]->name, threadNum ,FreePM_pipe[threadNum]->Hpipe);
M_CONNECT:
    rc = FreePM_pipe[threadNum]->Connect();
    if(rc)
    {   debug(0, 0) ("WARNING: Error connecting pipe %s: %s\n",FreePM_pipe[threadNum]->name,xstdio_strerror());
        goto ENDTHREAD;
    }
    debug(0, 2) ("Fs_ClientWork%i: Connecting pipe: %s Ok\n",threadNum,FreePM_pipe[threadNum]->name);
    LSthreads.state[threadNum] = 1;
    rc = FreePM_pipe[threadNum]->HandShake();
    if(rc)
    {   debug(0, 0) ("WARNING: Error HandShake pipe %s: %s\n",FreePM_pipe[threadNum]->name,xstdio_strerror());

        rc = DosDisConnectNPipe(FreePM_pipe[threadNum]->Hpipe);
        goto M_CONNECT;
    }
    debug(0, 2) ("Fs_ClientWork%i: HandShake pipe: %s Ok\n",threadNum,FreePM_pipe[threadNum]->name);

/***********/
   for(i = 0; i < FREEPMS_MAX_NUM_THREADS; i++)
      {
//    debug(0, 0) ("(%i %i)",i,LSthreads.thread_id[i]);
         if(LSthreads.thread_id[i] == -1)
         {     break;
         }
     }
   idd = LSthreads.next_free;
//    debug(0, 0) ("idd=%i",idd);
   ThreadStart = 1;

    LSthreads.state[threadNum] = 2;

    id = _beginthread(Fs_ClientWork,NULL, THREAD_STACK_SIZE,&idd);

    do
    { DosSleep(1);
    } while(ThreadStart == 1);

   while(__lxchg(&LSthreads.semaphore, LOCKED)) DosSleep(1);

   LSthreads.Nclients++;     // число живых клиентов
   LSthreads.working = 1;    // =0 на старте, =1 после появления первого живого клиента (защелка)
   LSthreads.num++;
   LSthreads.thread_id[LSthreads.next_free] = id;


   for(i = 0; i < FREEPMS_MAX_NUM_THREADS; i++)
      {
// l = (i + LSthreads.next_free)%MAX_NUM_THREADS;
//    debug(0, 0) ("[%i %i]",i,LSthreads.thread_id[i]);
         if(LSthreads.thread_id[i] == -1)
         {     LSthreads.next_free = i;
               break;
         }
      }

   ThreadStart = 3;
   __lxchg(&LSthreads.semaphore, UNLOCKED);
   DosSleep(1);
   debug(0, 2)("Fs_ClientWork%i: Pipe working %s\n",threadNum,FreePM_pipe[threadNum]->name);
/*****************/
   do
   {  
      LSthreads.state[threadNum] = 3;
      rc = _F_RecvCmdFromClient((void *)FreePM_pipe[threadNum], &ncmd,&data);

      if(rc)
      {  if(rc == -1)
         {  
            rc = FreePM_pipe[threadNum]->QueryState();
            if(rc == ERROR_BAD_PIPE || rc == ERROR_PIPE_NOT_CONNECTED)
                                  break; // клиент подох ??
         }
         debug(0, 9)("WARNING: Fs_ClientWork: RecvCmdFromClient error %x %s\n",rc,xstdio_strerror()); 
         goto ENDTHREAD;
      }
      LSthreads.state[threadNum]=1;

      LSthreads.state[threadNum] = 4;
      debug(0, 9)("Fs_ClientWork: Get ncmd %x %x\n",ncmd, data);
 
      // handle the command
      handler((void *)FreePM_pipe[threadNum], ncmd, data, threadNum);

   } while(ncmd);
/*****************/

ENDTHREAD:
    LSthreads.state[threadNum]= 5;
    debug(0, 2) ("Fs_ClientWork%i: Pipe Closing %s %x %x\n",threadNum,FreePM_pipe[threadNum]->name, threadNum ,FreePM_pipe[threadNum]->Hpipe);
//     DosSleep(3000);
//     rc = DosDisConnectNPipe(FreePM_pipe[threadNum].Hpipe);
//     rc += DosClose(FreePM_pipe[threadNum].Hpipe);
    rc = FreePM_pipe[threadNum]->Close();
    debug(0, 2) ("Fs_ClientWork%i: Pipe Close %s, Thread %i, ThId=%i, rc=%x\n",
                    threadNum,FreePM_pipe[threadNum]->name, LSthreads.thread_id[threadNum],rc);
    fflush(stdout);

//todo: call timeout_handler to kill windows with closed habs
   while(__lxchg(&LSthreads.semaphore, LOCKED))
                                       DosSleep(1);

//   sgLogError("End thread %i\n",threadNum);
   LSthreads.num--;
   LSthreads.thread_id[threadNum] = -1;
   LSthreads.state[threadNum] = -1;
   if(LSthreads.thread_id[LSthreads.next_free] != -1 || LSthreads.next_free > threadNum)
                                                               LSthreads.next_free = threadNum;
    if(LSthreads.n < threadNum)
             LSthreads.n = threadNum;
   for(i=0; i < FREEPMS_MAX_NUM_THREADS; i++)
      {
// l = (i + LSthreads.next_free)%MAX_NUM_THREADS;
//    debug(0, 0) ("[%i %i]",i,LSthreads.thread_id[i]);
         if(LSthreads.thread_id[i] == -1)
         {     LSthreads.next_free = i;
               break;
         }
      }

   LSthreads.Nclients--;     // число живых клиентов
   __lxchg(&LSthreads.semaphore, UNLOCKED);

    DosSleep(1);

}


extern "C" APIRET APIENTRY _startServerThreads(void *handl)
{  
  int i,ii, id, idd;
  typedef void (*handler_t) (void *obj, int ncmd, int data, int threadNum);

//   id = _beginthread(TestTest,NULL, STACK_SIZE*16,NULL);
//DosSleep(1000);
//DosSleep(1000);
// return;
   handler = (handler_t)handl;

   for(i=0;i < FREEPMS_MAX_NUM_THREADS;i++)
   { LSthreads.thread_id[i] = -1;
   }
   LSthreads.next_free = 0;
M0:
   LSthreads.working = 0; // нужно для организации задержки ожидания запуска первого клиента

   idd = LSthreads.next_free;
   ThreadStart = 1;

   id = _beginthread(&Fs_ClientWork,NULL, THREAD_STACK_SIZE * 2, (void *) &idd);
   // ((void*) (void*))
   //id = _beginthread( ((void(*) (void*))fix_asm_Fs_ClientWork), NULL, hack_thread_size, (void *) &idd);

   do
   { DosSleep(1);
   } while(ThreadStart == 1);

   while(__lxchg(&LSthreads.semaphore, LOCKED)) DosSleep(1);
   LSthreads.num++;
   LSthreads.thread_id[LSthreads.next_free] = id;
   for(i = 0; i < FREEPMS_MAX_NUM_THREADS; i++)
      {  ii = (i + LSthreads.next_free) % FREEPMS_MAX_NUM_THREADS;
         if(LSthreads.thread_id[ii] == -1)
         {     LSthreads.next_free = ii;
               break;
         }
      }
   ThreadStart = 3;

   __lxchg(&LSthreads.semaphore, UNLOCKED);

   return 0;
}

