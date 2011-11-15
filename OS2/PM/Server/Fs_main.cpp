/* Fs_main.cpp */
/* main() for server side of FreePM */
/* DEBUG: section 1     main server */
/* ver 0.00 20.08.2002       */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 #include <signal.h>
 #include <unistd.h>
#include <float.h>
#include <time.h>
#include <process.h>

#include "FreePM.hpp"
#include "FreePMs.hpp"


/*#include "FreePM_win.hpp" Maybe Constants ? */
//#include "FreePM_winConstants.hpp"
#include "F_pipe.hpp"
#include "FreePM_cmd.hpp"
#include "Fs_WND.hpp"
  #define FREEPMS_HPS
#include "Fs_HPS.hpp"
  #undef FREEPMS_HPS
#include "F_hab.hpp"
#include "Fs_config.hpp"
#include "Fs_driver.h"

//#include <sys/time.h>
#include "F_utils.hpp"
/* #include "snprintf.h" */


/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/
time_t getCurrentTime(void);

/*+---------------------------------+*/
/*| Internal function prototypes.   |*/
/*+---------------------------------+*/
extern "C" void   FreePM_cleanup(void);
extern "C" void   FreePM_cleanupHandler(int sig);
extern "C" void   FreePM_cleanupHandlerFP(int sig);

int startServerThreads(void);
int SetupSemaphore(void);
int SetupSignals(void);

/*+---------------------------------+*/
/*| Global variables                |*/
/*+---------------------------------+*/
#include "F_globals.hpp"
#include "Fs_globals.hpp"
const char *const _FreePM_Application_Name = FREEPM_SERVER_APPLICATION_NAME;
const char *const _FreePM_Application_Vers = FREEPM_VERSION;
class F_ServerConfig FPMs_config;

/*+---------------------------------+*/
/*| Static  variables               |*/
/*+---------------------------------+*/
static HMTX    FREEPM_hmtx     = NULLHANDLE; /* Mutex semaphore handle */

static int nxDefault=800, nyDefault=600,  bytesPerPixelDefault=4;
/*+---------------------------------+*/
/*|     local constants.            |*/
/*+---------------------------------+*/
/* server commands: todo */
#define S_SHUTDOWN    0x101
#define S_RECONFIGURE 0x102
#define S_ROTATE      0x103
#define S_DEBUG       0x104
#define S_KILL        0x105

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

struct LS_threads  LSthreads = { 0,0,0,0,0,0 };

class FreePM_session session;

int tst__lxchg(int volatile * a, int  b);
extern void fix_asm_Fs_ClientWork(void *);
void /*_Optlink*/  Fs_ClientWork( void * /* *param */);

//
// Main function
//
// General logic:
//   Setup/check semaphore (to prevent dupes)
//   Setup signals handler
//   Read config
//   Load output device driver
//   Start server threads
//   Add desktop
//   Infinite loop

int main(int narg, char *arg[], char *envp[])
{
  int i,j,rc;
  HMODULE hDeviceLib;

/* semaphore setup */
  rc = SetupSemaphore();
  if(rc)
  {
    if(rc == 2)
          printf("%s %s already running\n",_FreePM_Application_Name, _FreePM_Application_Vers);
      exit(rc);
  }
  atexit(&FreePM_cleanup);
  SetupSignals();

  rc = QueryProcessType();
  if(rc == 4)
    _FreePM_detachedMode = 1;

/* init time */
  getCurrentTime();
  _FreePM_start = _FreePM_current_time;

/* init debug */
  _db_init(_FreePMconfig.Log.log, FPMs_config.debugOptions);

  if(_FreePM_detachedMode)
  {
    debug(1, 0) ("Starting in detached mode %s version %s...\n",_FreePM_Application_Name, _FreePM_Application_Vers);
  } else {
    debug(1, 0) ("Starting %s version %s...\n",_FreePM_Application_Name, _FreePM_Application_Vers);
  }

/* read config */
  FPMs_config.Read("FreePMServer.ini");
/* init debug again */
  _db_init(_FreePMconfig.Log.log, FPMs_config.debugOptions);

/* load device driver */

  debug(1, 0) ("Loading driver %s\n",FPMs_config.deviceName);

  if (!DosLoadModule(NULL, 0, FPMs_config.deviceName, &hDeviceLib))
  {
    debug(1, 0) ("Module loaded\n");
    if (DosQueryProcAddr(hDeviceLib, 0, "FPM_DeviceStart",
                        (PFN*)&FPM_DeviceStart))
    {
      debug(1, 0) ("Error initialize driver\n");
      exit(1);
    };
    debug(1, 0) ("Address found\n");

  } else {
    debug(1, 0) ("Error loading driver module\n");
    exit(1);
  }

/* init pipes  */

  startServerThreads();

//  rc = session.AddDesktop(FPM_DEV_PMWIN,
//                           _FreePMconfig.desktop.nx,
//                           _FreePMconfig.desktop.ny,
//                           _FreePMconfig.desktop.bytesPerPixel,
//                           &_FreePMconfig.desktop.pp);
  printf("nx=%lu, ny=%lu, bpp=%lu\n", FPMs_config.DesktopNx,
                                      FPMs_config.DesktopNy,
                                      FPMs_config.BytesPerPel);
  rc = session.AddDesktop(FPM_DEV_PMWIN,
                           FPMs_config.DesktopNx,
                           FPMs_config.DesktopNy,
                           FPMs_config.BytesPerPel,
                           &_FreePMconfig.desktop.pp);

  debug(1, 1)("session.AddDesktop rc=%i\n",rc);

  debug(1, 1)("SRV main: Main idle loop\n");
/* Main idle loop */
        /*
  debug(1, 1)("LSthreads.n %d\n", LSthreads.n);
  debug(1, 1)("LSthreads.Nclients %d\n", LSthreads.Nclients);
  debug(1, 1)("LSthreads.n %d\n", LSthreads.n); */

  for(i=0; ;i++)
  {
    for(j=0; j< LSthreads.n; j++)
    {
      debug(1, 1)("(%i,%i) ",LSthreads.thread_id[j],LSthreads.state[j] );
    }
    debug(1, 1)("%i   \r",LSthreads.Nclients);

    fflush(stdout);
    DosSleep(100);
  }

  rc = session.DelDesktop(0);

  debug(1, 1)("session.DelDesktop(0) rc=%i\n",rc);

  debug(1, 1) ("Normal Shutting down...\n");
  exit(0);
}

/**************************************/
void /*_Optlink*/  FreePM_cleanup(void)
{   int rc;

//   WriteParameters();

//    lprgs.Write(szConfigFname);

/* в последнюю очередь освобождаем семафор */
    if(FREEPM_hmtx)
    {   rc = DosReleaseMutexSem(FREEPM_hmtx);        /* Relinquish ownership */
        rc = DosCloseMutexSem(FREEPM_hmtx);          /* Close mutex semaphore */
    }
}

/* rc = 0 (NO_ERROR) - Ok
      = 1 - error
      = 2 - DUPLICATE_NAME
*/
int SetupSemaphore(void)
{
 PID     pidOwner = 0;          /* PID of current mutex semaphore owner */
 TID     tidOwner = 0;          /* TID of current mutex semaphore owner */
 ULONG   ulCount  = 0;          /* Request count for the semaphore */
 APIRET  rc       = NO_ERROR;   /* Return code */
char *pstr;
pstr = FREEPM_MUTEX_NAME;
    rc = DosCreateMutexSem(FREEPM_MUTEX_NAME,      /* Semaphore name */
                           &FREEPM_hmtx, 0, FALSE);  /* Handle returned */
    if (rc != NO_ERROR)
    {
       if(rc == ERROR_DUPLICATE_NAME)
       {  return 2;
       } else {
          printf("DosCreateMutexSem error: return code = %u\n", rc);
          return 1;
       }
    }
//  if(!detachedMode)
//       printf("DosCreateMutexSem %i \n",__LINE__);
         /* This would normally be done by another unit of work */
    rc = DosOpenMutexSem(FREEPM_MUTEX_NAME,      /* Semaphore name  */
                         &FREEPM_hmtx);          /* Handle returned */
    if (rc != NO_ERROR) {
       printf("DosOpenMutexSem error: return code = %u\n", rc);
       return 1;
     }
//  if(!detachedMode)
//       printf("DosOpenMutexSem %i\n",__LINE__);

    rc = DosRequestMutexSem(FREEPM_hmtx,      /* Handle of semaphore */
                            (ULONG) 1000);  /* Timeout  */
    if (rc != NO_ERROR) {
       printf("DosRequestMutexSem error: return code = %u\n", rc);
       return 1;
    }
//  if(!detachedMode)
//      printf("DosRequestMutexSem %i\n",__LINE__);

    rc = DosQueryMutexSem(FREEPM_hmtx,         /* Handle of semaphore */
                          &pidOwner,    /* Process ID of owner */
                          &tidOwner,    /* Thread ID of owner */
                          &ulCount);    /* Count */
    if (rc != NO_ERROR) {
       printf("DosQueryMutexSem error: return code = %u\n", rc);
       return 1;
    } else if (!_FreePM_detachedMode)  {
       printf("Semaphore owned by PID %u, TID %u.", pidOwner, tidOwner);
       printf("  Request count is %u.\n", ulCount);
    } /* endif */

//  if(!detachedMode)
//       printf("DosQueryMutexSem %i\n",__LINE__);

 return NO_ERROR;
}


int SetupSignals(void)
 {
    if (SIG_ERR == signal(SIGABRT, FreePM_cleanupHandler)) {
       perror("Could not set SIGABRT");
       return EXIT_FAILURE;
    }

    if (SIG_ERR == signal(SIGBREAK, FreePM_cleanupHandler)) {
       perror("Could not set SIGBREAK");
       return EXIT_FAILURE;
    }

    if (SIG_ERR == signal(SIGINT, FreePM_cleanupHandler)) {
       perror("Could not set SIGINT");
       return EXIT_FAILURE;
    }

   if (SIG_ERR == signal(SIGFPE, FreePM_cleanupHandlerFP)) {
       perror("Could not set SIGFPE");
       return EXIT_FAILURE;
    }
    if (SIG_ERR == signal(SIGSEGV, FreePM_cleanupHandler)) {
       perror("Could not set SIGSEGV");
       return EXIT_FAILURE;
    }
    if (SIG_ERR == signal(SIGILL, FreePM_cleanupHandler)) {
       perror("Could not set SIGILL");
       return EXIT_FAILURE;
    }
    return 0;
 }

void showSigString(int s) {
        char * str="";
        switch (s) {
                case SIGABRT:  str="SIGABRT"; break;
                case SIGBREAK: str="SIGBREAK"; break;
                case SIGINT:  str="SIGINT"; break;
                case SIGFPE:  str="SIGFPE"; break;
                case SIGSEGV: str="SIGSEGV"; break;
                case SIGILL:  str="SIGILL"; break;
        }
        printf("Signal: %d = %s\n", s, str);
}

extern "C"  void   FreePM_cleanupHandler(int sig)
 {
    char str[100];
    ULONG Wrote;

    _FreePM_NeedToExit = 1; /* всем остальным ниткам надо срочно сваливать */
    sprintf(str, "\nSignal occurred %x.\n\r", sig);
        showSigString(sig);
    DosWrite(2, (PVOID)str, strlen(str), &Wrote);

    DosSleep(10);
    DosBeep(3000,5);
    DosSleep(10);
    DosBeep(4200,4);
    exit(1);
 }

/* На случай ошибки fp      */
extern "C"  void   FreePM_cleanupHandlerFP(int sig)
{  static int raz=0;
  _fpreset();

  if(++raz  > 1000000)
           _FreePM_NeedToExit = 1; /* всем остальным ниткам надо срочно сваливать */

    debug(1, 0)("FP Signal occurred %i",sig);
    DosBeep(5000,10);
//    DosWrite(2, (PVOID)str, strlen(str), &Wrote);
//    exit(10);
   if (SIG_ERR == signal(SIGFPE, FreePM_cleanupHandler)) {
       perror("Could not set SIGFPE");
       debug(1, 0)("Could not set SIGFPE");
       exit(10);
    }
  if(++raz  > 2000000)
       exit(20);

}

//!!!!!!!!!!!!!!! Move this code to Fs_PMdev !!!!!!!!!!
#define APIENTRY  _System
extern "C" LONG   APIENTRY GpiLine(HPS hps, PPOINTL pptlEndPoint);
extern "C" BOOL   APIENTRY GpiMove(HPS hps, PPOINTL pptlPoint);
extern "C" BOOL   APIENTRY GpiSetColor(HPS hps, LONG lColor);
extern HPS     hpsDrawBMPBuffer = NULLHANDLE;

// This functions must draw into bitmap which painted on WM_PAINT event
BOOL F_PS_GpiSetColor(struct  F_PS *ps, LONG lColor)
{
  GpiSetColor(hpsDrawBMPBuffer, lColor);
  return 0;
}

BOOL F_PS_GpiLine(struct  F_PS *ps, PPOINTL pptlPoint)
{
  POINTL ptl[4] = { 0, 0, 100, 100, 0, 100, 100, 0 };
  GpiLine(hpsDrawBMPBuffer, pptlPoint);
  return 0;
}

BOOL F_PS_GpiMove(struct  F_PS *ps, PPOINTL pptlPoint)
{
  POINTL ptl[4] = { 0, 0, 100, 100, 0, 100, 100, 0 };
  GpiMove(hpsDrawBMPBuffer, pptlPoint);
  return 0;
}

HPS     APIENTRY  F_WinGetPS(HWND hwnd) { return 0; }


/*************************************************************************/
/*
int tst__lxchg(int volatile * a, int  b) {
        //printf("FIXME: builtin.h:%s (%d, %d)\n", __func__, *a, b);
        //DosEnterCritSec();
        int ret = 1;
        if(*a == 0 && b == 1) {
                *a = b;
                ret = 0;
        }
        if(*a == 1 && b == 0) {
                //*a = b;
                ret = 1; //0
        }

        //DosExitCritSec();
        return ret;
}
*/


class NPipe FreePM_pipe[FREEPMS_MAX_NUM_THREADS];

int ThreadStart = 0;



void /*_Optlink*/  Fs_ClientWork( void * /* *param */)
{


    int i,rc, threadNum,id,idd;
    int ncmd,data,l, len;
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
        /*int ret = 1;
        while(ret) {
        ret = 1;
        if(LSthreads.semaphore == 0 && LOCKED == 1) {
                LSthreads.semaphore = LOCKED;
                ret = 0;
        }
        if(LSthreads.semaphore == 1 && LOCKED == 0) {
                //*a = b;
                ret = 1; //0
        }
        //DosSleep(1);
        //sleep(1);
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
    debug(0, 2) ("Fs_ClientWork%i: Pipe creating  (%i)\n",threadNum,LSthreads.thread_id[threadNum]);
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


    FreePM_pipe[threadNum]  = NPipe(FREEPM_BASE_PIPE_NAME,SERVER_MODE,FREEPMS_MAX_NUM_THREADS,threadNum);

    rc = FreePM_pipe[threadNum].Create();
    if(rc == ERROR_TOO_MANY_OPEN_FILES)
    {     rc = OS2SetRelMaxFH(8);
          rc = FreePM_pipe[threadNum].Create();
    }

    if(rc)
    {  snprintf(str,256, "Error pipe creating  %s rc=%i",FreePM_pipe[threadNum].name,rc);
       if(rc == ERROR_INVALID_PARAMETER)
                   strcat(str,"(INVALID PARAMETER)");
       else
          if (rc == ERROR_PIPE_BUSY)
                   strcat(str,"(PIPE_BUSY)");
       if (rc == ERROR_PIPE_BUSY)
       {    debug(0, 0) ("WARNING: Fs_ClientWork: %s\n",str);
            goto ENDTHREAD;
       }
       fatal(str);
    }
    LSthreads.state[threadNum] = 0;
    debug(0, 2) ("Fs_ClientWork%i: Pipe create %s %x %x\n",threadNum,FreePM_pipe[threadNum].name, threadNum ,FreePM_pipe[threadNum].Hpipe);
M_CONNECT:
    rc = FreePM_pipe[threadNum].Connect();
    if(rc)
    {   debug(0, 0) ("WARNING: Error connecting pipe %s: %s\n",FreePM_pipe[threadNum].name,xstdio_strerror());
        goto ENDTHREAD;
    }
    debug(0, 2) ("Fs_ClientWork%i: Connecting pipe: %s Ok\n",threadNum,FreePM_pipe[threadNum].name);
    LSthreads.state[threadNum] = 1;
    rc = FreePM_pipe[threadNum].HandShake();
    if(rc)
    {   debug(0, 0) ("WARNING: Error HandShake pipe %s: %s\n",FreePM_pipe[threadNum].name,xstdio_strerror());

        rc = DosDisConnectNPipe(FreePM_pipe[threadNum].Hpipe);
        goto M_CONNECT;
    }
    debug(0, 2) ("Fs_ClientWork%i: HandShake pipe: %s Ok\n",threadNum,FreePM_pipe[threadNum].name);

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
    debug(0, 2) ("Fs_ClientWork%i: Pipe working %s\n",threadNum,FreePM_pipe[threadNum].name);
/*****************/
   do
   {  LSthreads.state[threadNum] = 3;
      rc = FreePM_pipe[threadNum].RecvCmdFromClient(&ncmd,&data);
      if(rc)
      {  if(rc == -1)
         {  rc = FreePM_pipe[threadNum].QueryState();
            if(rc == ERROR_BAD_PIPE || rc == ERROR_PIPE_NOT_CONNECTED)
                                  break; // клиент подох ??
         }
         debug(0, 0) ("WARNING: Fs_ClientWork: RecvCmdFromClient error %x %s\n",rc,xstdio_strerror());
         goto ENDTHREAD;
      }
      LSthreads.state[threadNum]=1;

    LSthreads.state[threadNum] = 4;
   debug(0, 9) ("Fs_ClientWork: Get ncmd %x %x\n",ncmd, data);
      switch(ncmd)
      {
        case F_CMD_GET_IHAB:
         {  int iHAB = 1, iClientId = 2,inf[2];
             debug(0, 2) ("Fs_ClientWork: F_CMD_GET_IHAB not yet full implemented\n");

             iHAB = session.hab_list.Add(iClientId,threadNum);
             inf[0] = iHAB;
             inf[1] = iClientId;
           rc=  FreePM_pipe[threadNum].SendDataToServer(inf , sizeof(inf));
         }
           break;
        case F_CMD_CLIENT_EXIT:
//todo: clear ihab = data
             debug(0, 2) ("Fs_ClientWork: F_CMD_CLIENT_EXIT %x ; not yet implemented\n",data);
           break;
        case F_CMD_WINPOSTMSG:
         {  SQMSG sqmsg;
            int rc1,ihabto;
             rc = FreePM_pipe[threadNum].RecvDataFromClient(&sqmsg, &l, sizeof(SQMSG));
             debug(0, 2) ("Fs_ClientWork: F_CMD_WINPOSTMSG get %i bytes\n",l);
             if(rc == 0 && l == sizeof(SQMSG))
             {   rc1 = _WndList.QueryHab(sqmsg.qmsg.hwnd, ihabto);
                 if(rc1 == 1)
                 { sqmsg.ihto = ihabto;
                   rc  = session.hab_list.Queue.Add(&sqmsg);
                 }
             }
         }
           break;
        case F_CMD_WINSENDMSG:
         {  SQMSG sqmsg;
            int rc1,ihabto;
             rc = FreePM_pipe[threadNum].RecvDataFromClient(&sqmsg, &l, sizeof(SQMSG));
             debug(0, 2) ("Fs_ClientWork: F_CMD_WINPOSTMSG get %i bytes\n",l);
             if(rc == 0 && l == sizeof(SQMSG))
             {   rc1 = _WndList.QueryHab(sqmsg.qmsg.hwnd, ihabto);
                 if(rc1 == 1)
                 { sqmsg.ihto = ihabto;
//todo: it is sendmsg == postmsg implementation
//it seems that we must wait in client via call to CMD_WINQUERYPOSTED or smth like it
                   rc  = session.hab_list.Queue.Add(&sqmsg);
                 }
             }
         }
           break;

        case F_CMD_WINQUERY_MSG: /* Query messages for ihab = data */
          {  int nmsg=0;
             int ihabto;
             ihabto  = data;
             nmsg  = session.hab_list.Queue.QueryNmsg(ihabto);
             if(nmsg)
                debug(0, 2) ("Fs_ClientWork: F_CMD_WINQUERY_MSG, nmsg=%i\n",nmsg);
             rc=  FreePM_pipe[threadNum].SendDataToServer(&nmsg , sizeof(int));
          }
           break;
        case F_CMD_WINGET_MSG:  /* Get message for ihab = data */
          {  SQMSG sqmsg;
             int rc, ihabto;
             int nmsg=0;
             ihabto = data;
             rc = session.hab_list.Queue.GetForIhab(&sqmsg, ihabto);
             if(rc == 0) nmsg = 1;

           if(nmsg == 1)
             debug(0, 2) ("Fs_ClientWork: F_CMD_WINGET_MSG for %i, msg=%x\n",ihabto, sqmsg.qmsg.msg);
           else
             debug(0, 2) ("Fs_ClientWork: F_CMD_WINGET_MSG, No msg  for %i\n",ihabto);
             rc=  FreePM_pipe[threadNum].SendDataToServer(&nmsg , sizeof(int));
             if(nmsg)
             { rc=  FreePM_pipe[threadNum].SendDataToServer(&sqmsg.qmsg , sizeof(QMSG));
             }
          }
           break;

        case F_CMD_WINCREATE_HWND:
         { HWND hwnd;
           int iHab;
           iHab = data;
           hwnd = _WndList.Add(iHab);
//todo: set _WndList with thread info and cleanup at thread(client exit) end
           rc=  FreePM_pipe[threadNum].SendDataToServer(&hwnd , sizeof(HWND));
         }

           break;
        case F_CMD_WINSET_PARENT_HWND:
          {   HWND hwnd, hwndParent;
              hwnd = data;
              rc = FreePM_pipe[threadNum].RecvDataFromClient((void *)&hwndParent, &len, sizeof(HWND));
              rc = _WndList.SetParent(hwnd, hwndParent);
          }
           break;

        case F_CMD_GET_HPS:
          {   HWND hwnd;
              HPS hps;
              hwnd = data;
              hps = _WndList.GetHPS(hwnd);
              rc=  FreePM_pipe[threadNum].SendDataToServer(&hps, sizeof(HPS));
          }
           break;
        case F_CMD_RELEASE_HPS:
          {
              HPS hps;
              int rc1;
              hps = data;
              rc1 = _WndList.ReleaseHPS(hps);
              rc=  FreePM_pipe[threadNum].SendDataToServer(&rc1, sizeof(int));
          }
           break;
        case F_CMD_WIN_SET_WND_SIZE:
          {   HWND hwnd = data;
              int par[2],rc1;
              rc = FreePM_pipe[threadNum].RecvDataFromClient((void *)&par[0], &len, sizeof(int)*2);
              rc1 = 0;
              //todo
              rc=  FreePM_pipe[threadNum].SendDataToServer(&rc1, sizeof(int));
          }
           break;

        case F_CMD_WIN_SET_WND_POS:
          {   HWND hwnd = data;
              int par[2],rc1;
              rc = FreePM_pipe[threadNum].RecvDataFromClient((void *)&par[0], &len, sizeof(int)*2);
              rc1 = 0;
              //todo
              rc=  FreePM_pipe[threadNum].SendDataToServer(&rc1, sizeof(int));
          }
           break;

        case F_CMD_WIN_GET_WND_SWP:
          {   HWND hwnd = data;
              SWP swp;
              //todo
              rc=  FreePM_pipe[threadNum].SendDataToServer(&swp, sizeof(SWP));
          }
           break;

        case F_CMD_WIN_SHOW:
          {   HWND hwnd = data;
              int par,rc1;
              rc = FreePM_pipe[threadNum].RecvDataFromClient((void *)&par, &len, sizeof(int));
              rc1 = 0;
              //todo
              rc=  FreePM_pipe[threadNum].SendDataToServer(&rc1, sizeof(int));
          }
           break;

        case F_CMD_GPI_SET_COLOR:
          {   HPS hps;
              int color, len, rc1=FALSE;
              hps = data;
              rc = FreePM_pipe[threadNum].RecvDataFromClient((void *)&color, &len, sizeof(int));
              debug(0, 0) ("F_CMD_GPI_SET_COLOR: get hps=%x, color=%x, len=%i\n",hps,color,len);
              if(hps >= 0 && hps < _WndList.numPS)
              {  if(_WndList.pPS[hps].used)
                                        /* rc1 = F_GpiSetColor(&_WndList.pPS[hps], color); */
                     rc1 = F_PS_GpiSetColor(&_WndList.pPS[hps], color);
              }
              rc=  FreePM_pipe[threadNum].SendDataToServer(&rc1, sizeof(int));
          }
           break;
        case F_CMD_GPI_MOVE:
          {   HPS hps;
              int rc1=FALSE;
              POINTL Point;
              hps = data;
              rc = FreePM_pipe[threadNum].RecvDataFromClient((void *)&Point, &len, sizeof(POINTL));
              debug(0, 0) ("F_CMD_GPI_MOVE: get hps=%x,len=%i\n",hps,len);

              if(hps >= 0 && hps < _WndList.numPS)
              {  if(_WndList.pPS[hps].used)
                    rc1 = F_PS_GpiMove(&_WndList.pPS[hps], &Point);
              }
              rc=  FreePM_pipe[threadNum].SendDataToServer(&rc1, sizeof(int));
          }
           break;
        case F_CMD_GPI_LINE:
          {   HPS hps;
              int rc1=FALSE;
              POINTL Point;
              hps = data;
              rc = FreePM_pipe[threadNum].RecvDataFromClient((void *)&Point, &len, sizeof(POINTL));

              if(hps >= 0 && hps < _WndList.numPS)
              {  if(_WndList.pPS[hps].used)
                    rc1 = F_PS_GpiLine(&_WndList.pPS[hps], &Point);
              }
              rc=  FreePM_pipe[threadNum].SendDataToServer(&rc1, sizeof(int));
          }
           break;
        case F_CMD_GPI_DRAW_LINE:
//TODO
           break;
        case F_CMD_GPI_DRAW_RECT:
//TODO
           break;


#define POKA 0
#if POKA
     case 1:
             debug(0, 2) ("Fs_ClientWork: Get ncmd %x %x\n",ncmd, data);
             rc=FreePM_pipe[threadNum].SendCmdToServer(ncmd,data);
             switch(data)
             {  case S_SHUTDOWN:
//                 squid_signal(SIGTERM, shut_down, SA_NODEFER | SA_RESETHAND | SA_RESTART);
//            debug(0, 0) ("squid_signal:\n");
                 DosSleep(100);
                  rc = raise(SIGTERM);
//            debug(0, 0) ("raise: rc =%x %s\n",rc,xstdio_strerror());
                  break;
               case S_RECONFIGURE:
//                       do_reconfigure = 1;
                  break;
               case S_ROTATE:
//                       do_rotate = 1;
                   break;
               case S_DEBUG:
                  break;
               case S_KILL:
           debug(1, 1) ("Preparing for killing\n");
//               do_shutdown++;
//           SquidShutdown(NULL);
                  break;
             }
            ncmd = 0;
           break;
#endif
     //POKA
         default:
debug(0, 2) ("Fs_ClientWork:WARNING: Unimplemented cmd %x\n",ncmd);
            ncmd = 0;
           break;

      } //endof switch(ncmd)

   } while(ncmd);
/*****************/

ENDTHREAD:
    LSthreads.state[threadNum]= 5;
    debug(0, 2) ("Fs_ClientWork%i: Pipe Closing %s %x %x\n",threadNum,FreePM_pipe[threadNum].name, threadNum ,FreePM_pipe[threadNum].Hpipe);
//     DosSleep(3000);
//     rc = DosDisConnectNPipe(FreePM_pipe[threadNum].Hpipe);
//     rc += DosClose(FreePM_pipe[threadNum].Hpipe);
    rc = FreePM_pipe[threadNum].Close();
    debug(0, 2) ("Fs_ClientWork%i: Pipe Close %s, Thread %i, ThId=%i, rc=%x\n",
                    threadNum,FreePM_pipe[threadNum].name, LSthreads.thread_id[threadNum],rc);
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


int startServerThreads(void)
{  int i,ii, id, idd;
//   id = _beginthread(TestTest,NULL, STACK_SIZE*16,NULL);
//DosSleep(1000);
//DosSleep(1000);
// return;
   for(i=0;i < FREEPMS_MAX_NUM_THREADS;i++)
   { LSthreads.thread_id[i] = -1;
   }
   LSthreads.next_free = 0;
M0:
   LSthreads.working = 0; // нужно для организации задержки ожидания запуска первого клиента

   idd = LSthreads.next_free;
   ThreadStart = 1;


   id = _beginthread(&Fs_ClientWork,NULL, THREAD_STACK_SIZE, (void *) &idd);
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

