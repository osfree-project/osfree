/*
 $Id: Fc_main.cpp,v 1.7 2003/06/15 17:45:48 evgen2 Exp $
*/
/* Fc_main.cpp */
/* FreePM client side main*/
/* DEBUG: section 1     main client */
/* ver 0.01 22.08.2002       */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreePM.hpp"
#define F_INCL_DOSPROCESS
#define F_INCL_DOSNMPIPES
#define F_INCL_DOSSEMAPHORES
   #include "F_OS2.hpp"
#include "F_pipe.hpp"
#include "F_utils.hpp"
#include "Fc_config.hpp"

/*+---------------------------------+*/
/*| External function prototypes.   |*/
/*+---------------------------------+*/

/*+---------------------------------+*/
/*| Internal function prototypes.   |*/
/*+---------------------------------+*/
int InitServerConnection(char *remotemachineName);
int CloseServerConnection(void);

int _F_SendCmdToServer(int cmd, int data);
int _F_SendGenCmdToServer(int cmd, int par);
int _F_SendGenCmdDataToServer(int cmd, int par, void *data, int datalen);
int _F_SendDataToServer(void *data, int len);
int _F_RecvDataFromServer(void *data, int *len, int maxlen);

/*+---------------------------------+*/
/*| Global variables                |*/
/*+---------------------------------+*/
#include "F_globals.hpp"
class F_ClientConfig FPM_config;

const char *const _FreePM_Application_Name = FREEPM_CLIENT_APPLICATION_NAME;
const char *const _FreePM_Application_Vers = FREEPM_VERSION;

/*+---------------------------------+*/
/*| Static  variables               |*/
/*+---------------------------------+*/
static int nxDefault=800, nyDefault=600,  bytesPerPixelDefault=4;
static char *ExternMachine = NULL;   /* Name of external machine for pipes like \\MACHINE\PIPE\SQDR   */
//static char ExternMachineName[_MAX_FNAME]="";
/*+---------------------------------+*/
/*|     local constants.            |*/
/*+---------------------------------+*/
char PipeName[256];


//class FreePM_session session;
class NPipe *pF_pipe;
static volatile int AccessF_pipe = UNLOCKED;
#define LOCK_PIPE                               \
    {   int ilps_raz = 0, ilps_rc;              \
        do                                      \
        {  ilps_rc =  __lxchg(&AccessF_pipe,LOCKED);  \
           if(ilps_rc)                          \
           { if(++ilps_raz  < 3)  DosSleep(0);  \
             else            DosSleep(1);       \
           }                                    \
        } while(ilps_rc);          \
    }

#define UNLOCK_PIPE  {__lxchg(&AccessF_pipe,UNLOCKED);}

int InitServerConnection(char *remotemachineName)
{  int rc;

static  char buf[256];

/* init time */
    getCurrentTime();

    _FreePM_start = _FreePM_current_time;
/* First  let's look for FreePM.ini and read it if any */
   FPM_config.Read("FreePM.ini");
/* init debug */
    _db_init(_FreePMconfig.Log.log, FPM_config.debugOptions);

   rc = QueryProcessType();
   if(rc == 4)
       _FreePM_detachedMode = 1;

   if(_FreePM_detachedMode)
   { debug(1, 0) ("Starting in detached mode %s version %s...\n",_FreePM_Application_Name, _FreePM_Application_Vers);
   } else {
     debug(1, 0) ("Starting %s version %s...\n",_FreePM_Application_Name, _FreePM_Application_Vers);
   }
   if(remotemachineName)
   {  strcpy(FPM_config.ExternMachineName,remotemachineName);
      ExternMachine = &FPM_config.ExternMachineName[0];
   } else {
//       ExternMachineName[0] = 0;
/* test for FreePM's semaphore  at local machine */
      printf(" Fc_main: FREEPM_MUTEX_NAME=%s \n", FREEPM_MUTEX_NAME );
       HMTX    FREEPM_hmtx     = NULLHANDLE; /* Mutex semaphore handle */
       rc = DosOpenMutexSem(FREEPM_MUTEX_NAME,    /* Semaphore name */
                            &FREEPM_hmtx);        /* Handle returned */
       DosCloseMutexSem(FREEPM_hmtx);
       if(rc)
       {  /* FreePM server is not running at local machine, let's look for FreePM.ini */
           debug(1, 1)("FreePM server is not running at local machine, test for server at %s\n",FPM_config.ExternMachineName);
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
   
   debug(1, 0)("Pipe: FREEPM_BASE_PIPE_NAME=%s \n", FREEPM_BASE_PIPE_NAME);
   pF_pipe = new NPipe(PipeName,CLIENT_MODE);

   rc = pF_pipe->Open();
   if(rc)
   {
      debug(1, 0)("F_pipe.Open rc = %i (%s)\n",rc,GetOS2ErrorMessage(rc));
      if(ExternMachine)
      {   debug(1, 0)("%s is not running at remoute machine %s, exitting...\n",FREEPM_SERVER_APPLICATION_NAME,ExternMachine);
      } else {
          debug(1, 0)("%s is not running at local machine, exitting...\n",FREEPM_SERVER_APPLICATION_NAME);
      }
      fatal("FreePM server not running");
   }
    rc = pF_pipe->HandShake();
    if(rc ==  HAND_SHAKE_ERROR)
    {   debug(1, 0)("Error handshake %i, pipe %s\n",rc,PipeName);
        fatal("Error handshake  pipe");
    }
//todo
    rc = 0;
    return rc;
}

int CloseServerConnection(void)
{   if(pF_pipe)
    {
    LOCK_PIPE;
    debug(1, 0) ("CloseServerConnection\n");
       delete pF_pipe;
       pF_pipe = NULL;
    UNLOCK_PIPE;
     }

    return 0;
}

int _F_SendCmdToServer(int ncmd, int data)
{   int rc;
    LOCK_PIPE;

    rc = pF_pipe->SendCmdToServer( ncmd,  data);
    if(rc == ERROR_BROKEN_PIPE) /* attempt to reconnect till timeout */
    { int raz;
      debug(1, 1) (__FUNCTION__ " Broken Pipe, Try to reconnect...\n");
      for(raz = 0;raz < 10; raz++)
      {  delete pF_pipe;
         pF_pipe = NULL;
         DosSleep(100+raz*10);
         pF_pipe = new NPipe(PipeName,CLIENT_MODE);
         rc = pF_pipe->Open();
         if(rc)
         {    debug(1, 1) (__FUNCTION__ " Broken Pipe, Server still down...\n");
         } else {
           rc = pF_pipe->HandShake();
           if(rc ==  HAND_SHAKE_ERROR)
           {   debug(1, 0)("Error handshake %i, pipe %s\n",rc,PipeName);
           } else {
               break;
           }
         }
      }
    }

    debug(1, 8) ("Send cmd %x %x, rc=%i\n",ncmd, data, rc);

    UNLOCK_PIPE;
   return rc;
}

/* Послать команду cmd c параметром par на сервер, получить ответ и вернуть его */
/* застрелиться при ошибках */
int _F_SendGenCmdToServer(int cmd, int par)
{   int rc,len,retcode;
    rc = _F_SendCmdToServer(cmd, par);
    if(rc)
    {  debug(1, 0)("WARNING:"__FUNCTION__":SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("SendCmdToServer Error\n");
    }
    rc = _F_RecvDataFromServer(&retcode, &len, sizeof(int));
    if(rc)
    {  debug(1, 0)("WARNING:"__FUNCTION__":RecvDataFromServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("RecvDataFromServer Error\n");
    }
    debug(1, 3)( __FUNCTION__ "(cmd=%x, par=%x) ret =%x\n",cmd, par, retcode);

    return retcode;
}

/* Послать команду cmd c параметром par, данные data длиной datalen*sizeof(int) на сервер, получить ответ и вернуть его */
/* застрелиться при ошибках */
int _F_SendGenCmdDataToServer(int cmd, int par, void *data, int datalen)
{   int rc,len,retcode;
M:  rc = _F_SendCmdToServer(cmd, par);
    if(rc)
    {  debug(1, 0)("WARNING:"__FUNCTION__":SendCmdToServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("SendCmdToServer Error\n");
    }
    rc =  _F_SendDataToServer(data, datalen * sizeof(int));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {   debug(1, 1)("WARNING:"__FUNCTION__":SendDataToServer return Broken Pipe, try to reconnect...\n");
           goto M;/* attempt to reconnect till timeout */
       }
       debug(1, 0)("WARNING:"__FUNCTION__":SendDataToServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("SendCmdToServer Error\n");
    }

    rc = _F_RecvDataFromServer(&retcode, &len, sizeof(int));
    if(rc)
    {  if(rc == ERROR_BROKEN_PIPE)
       {   debug(1, 1)("WARNING:"__FUNCTION__":SendDataToServer return Broken Pipe, try to reconnect...\n");
           goto M;/* attempt to reconnect till timeout */
       }
       debug(1, 0)("WARNING:"__FUNCTION__":RecvDataFromServer Error: %s\n",GetOS2ErrorMessage(rc));
       fatal("RecvDataFromServer Error\n");
    }
    debug(1, 3)( __FUNCTION__ " retcode =%x\n",retcode);

    return retcode;
}


int _F_SendDataToServer(void *data, int len)
{   int rc;
    LOCK_PIPE;

    rc = pF_pipe->SendDataToServer(data,  len);
     debug(1, 9) ("Send data %i bytes, rc=%i\n", len, rc);

    UNLOCK_PIPE;
   return rc;
}

int _F_RecvDataFromServer(void *data, int *len, int maxlen)
{   int rc;
    LOCK_PIPE;

    rc = pF_pipe->RecvDataFromClient(data, len, maxlen);
     debug(1, 9) ("Recv data %i bytes, rc=%i\n", *len, rc);

    UNLOCK_PIPE;
   return rc;
}

