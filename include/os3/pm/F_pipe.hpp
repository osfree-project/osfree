/* 
 $Id: F_pipe.hpp,v 1.1.1.2 2002/09/05 17:55:49 evgen2 Exp $ 
*/
/* F_pipe.hpp */
/* class NPipe, связь клиента с сервером */
#ifndef FREEPM_PIPES
  #define FREEPM_PIPES

#define F_INCL_DOSNMPIPES 
#include <F_base.hpp>
#include <builtin.h>
#define FREEPM_BASE_PIPE_NAME        "\\PIPE\\FREEPM"  /* Base pipe name */

#define MAX_NUM_PIPES 32

#define LOCKED    1
#define UNLOCKED  0

#define SERVER_MODE              1
#define CLIENT_MODE              2


#define REMOTE_PIPE              2
#define DISCON_MODE              3
#define BAD_INPUT_ARGS           99
#define MAX_PIPE_NAME_LEN        80
#define MAX_SERV_NAME_LEN        8
#define DEFAULT_MAKE_MODE        NP_ACCESS_DUPLEX
#define DEFAULT_PIPE_MODE        NP_WMESG | NP_RMESG | 0x01
#define DEFAULT_OPEN_FLAG        OPEN_ACTION_OPEN_IF_EXISTS
#define DEFAULT_OPEN_MODE        OPEN_FLAGS_WRITE_THROUGH | \
                                 OPEN_FLAGS_FAIL_ON_ERROR | \
                                 OPEN_FLAGS_RANDOM |        \
                                 OPEN_SHARE_DENYNONE |      \
                                 OPEN_ACCESS_READWRITE
#define DEFAULT_OUTB_SIZE        0x1000
#define DEFAULT_INPB_SIZE        0x1000
#define DEFAULT_TIME_OUTV        20000L
#define TOKEN_F3_DISCON          0x0000003DL

#define HAND_SHAKE_LEN           0x08
#define HAND_SHAKE_INP           "pIpEtEsT"
#define HAND_SHAKE_OUT           "PiPeTeSt"
#define HAND_SHAKE_ERROR         101
#define PROGRAM_ERROR            999

/*********************************************/
/* класс NPipe                               */
/*********************************************/

class NPipe
{
public:
   HPIPE   Hpipe;      /* the handle of the pipe */
   char name[256];     /* pipe name */
   ULONG   ulOpenMode; /*  A set of flags defining the mode in which to open the pipe. */
   ULONG   ulPipeMode; /*  A set of flags defining the mode of the pipe. */
   ULONG   ulOutBufSize ; /*  The number of bytes to allocate for the outbound (server to client) buffer. */
   ULONG   ulInpBufSize;  /*  The number of bytes to allocate for the inbound (client to server) buffer. */
   ULONG   ulTimeOut;     /*  The maximum time, in milliseconds, to wait for a named-pipe instance to become available. */
   int     mode;          /* SERVER_MODE - pабота как сеpвеp, CLIENT_MODE - как клиент */
   ULONG ulActionTaken;
   int    nInstanse;

   NPipe()
   {  Hpipe=NULL;
      name[0]=0;
      ulOpenMode   = DEFAULT_OPEN_MODE; /* DEFAULT_MAKE_MODE; for server */
      ulPipeMode   = DEFAULT_PIPE_MODE;
      ulOutBufSize = DEFAULT_OUTB_SIZE;
      ulInpBufSize = DEFAULT_INPB_SIZE;
      ulTimeOut    = DEFAULT_TIME_OUTV;
      mode = CLIENT_MODE;
      nInstanse = 0;
   }
   NPipe(char *_name, int _mode)
   {
      strcpy(name, _name);
      Hpipe=NULL;
      ulOpenMode   = DEFAULT_MAKE_MODE;
      ulPipeMode   = DEFAULT_PIPE_MODE;
      ulOutBufSize = DEFAULT_OUTB_SIZE;
      ulInpBufSize = DEFAULT_INPB_SIZE;
      ulTimeOut    = DEFAULT_TIME_OUTV;
      mode = _mode;
      if(mode == CLIENT_MODE)
              ulOpenMode   = DEFAULT_OPEN_MODE;
      else    ulOpenMode   = DEFAULT_MAKE_MODE;
   }

   NPipe(char *_name, int _mode, int maxnumInstance, int _nInstanse)
   {
      strcpy(name, _name);
      Hpipe=NULL;
      ulOpenMode   = DEFAULT_MAKE_MODE;
      ulPipeMode   = DEFAULT_PIPE_MODE;
      ulOutBufSize = DEFAULT_OUTB_SIZE;
      ulInpBufSize = DEFAULT_INPB_SIZE;
      ulTimeOut    = DEFAULT_TIME_OUTV;
      mode = _mode;
      if(mode == CLIENT_MODE)
              ulOpenMode   = DEFAULT_OPEN_MODE;
      else
      {   ulOpenMode   = DEFAULT_MAKE_MODE;
          ulPipeMode  = (ulPipeMode & ~0xff) | ( maxnumInstance & 0xff);
      }
      nInstanse = _nInstanse;
   }

   NPipe(char *_name, int OpenMode,int PipeMode,int OutBufSize,int InpBufsize,int Timeout, int _mode)
   {
      strcpy(name, _name);
      Hpipe=NULL;
      ulOpenMode   = OpenMode;
      ulPipeMode   = PipeMode;
      ulOutBufSize = OutBufSize;
      ulInpBufSize = InpBufsize;
      ulTimeOut    = Timeout;
      mode = _mode;
      if(mode == CLIENT_MODE)
              ulOpenMode   = DEFAULT_OPEN_MODE;
      else    ulOpenMode   = DEFAULT_MAKE_MODE;
   }

   ~NPipe()
   { if(Hpipe)
     {  DosClose(Hpipe);
     }
     Hpipe = NULL;
   }

   int Create(void)
   {  int rc;
      if(mode != SERVER_MODE)
               return -1;
      rc  = DosCreateNPipe(name,&Hpipe,
                           ulOpenMode,ulPipeMode,
                           ulOutBufSize,ulInpBufSize,
                           ulTimeOut);
      return rc;
   }
   int Connect(void)
   {  int rc;
      if(mode != SERVER_MODE)
               return -1;
      rc = DosConnectNPipe(Hpipe);
      return rc;
   }

   int DisConnect(void)
   { int rc = -1;
      if(mode != SERVER_MODE)
               return rc;
      rc = DosDisConnectNPipe(Hpipe);
     return rc;
   }

   int Close(void)
   { if(Hpipe)
     {  DosClose(Hpipe);
     }
     Hpipe = NULL;
     return 0;
   }

   int Open(void)
   {  int rc;
      if(mode != CLIENT_MODE)
               return -1;

      rc = DosOpen(name,
                   &Hpipe,
                   &ulActionTaken,
                   0,
                   0,
                   DEFAULT_OPEN_FLAG, /* ulOpenFlag, */
                   ulOpenMode,
                   0);
      return rc;
   }
   int HandShake(void)
   {  int rc;
      if(mode == SERVER_MODE)
         rc = HandShakeServer();
      else
         rc = HandShakeClient();
      return rc;
   }


   int HandShakeClient(void)
   {  int rc,rc0;
      char str[256];
      ULONG ulBytesDone;

      rc = 1;

      rc0 = DosWrite(Hpipe,
                     HAND_SHAKE_INP,
                     strlen(HAND_SHAKE_INP)+1, /* с нулем на конце */
                     &ulBytesDone);
//printf("Клиент handshake -> %s\n",HAND_SHAKE_INP);
      if (!rc0)
      {  str[0] = 0;
         rc0 = DosRead(Hpipe,str,
                       (ULONG)strlen(HAND_SHAKE_OUT)+1,
                        &ulBytesDone);
//printf("Клиент handshake <- %s\n",str);
         if (strcmp(str,
                    HAND_SHAKE_OUT))
         {  rc = HAND_SHAKE_ERROR;
         } else rc =0;
      }
      return rc;
   }

   int HandShakeServer(void)
   {  int rc,rc0;
      char str[256];
      ULONG ulBytesDone;

      rc = 1;
      str[0] = 0;
      rc0 = DosRead(Hpipe,str,
                       (ULONG)strlen(HAND_SHAKE_INP)+1,
                        &ulBytesDone);
//printf("Сеpвеp handshake <- %s\n",str);

      if (strcmp(str, HAND_SHAKE_INP))
      {  rc = HAND_SHAKE_ERROR;
      } else {
         rc0 = DosWrite(Hpipe,
                     HAND_SHAKE_OUT,
                     strlen(HAND_SHAKE_OUT)+1, /* с нулем на конце */
                     &ulBytesDone);
//printf("Сеpвеp handshake -> %s\n",HAND_SHAKE_OUT);
         if(!rc0)
               rc =0;

      }
      return  rc;
   }
/* послать команду ncmd с данными data */
   int SendCmdToServer(int ncmd, int data)
   {  char str[32];
      int rc, *pdata;
      ULONG ulBytesDone;

      pdata = (int *)&str[0];
      *pdata = ncmd;
      pdata[1] = data;
      rc = DosWrite(Hpipe,(void *)pdata,sizeof(int)*2, &ulBytesDone);
      if(ulBytesDone != sizeof(int)*2  && rc == 0)
         rc = -1;
      return rc;
   }

   int SendDataToServer(void *data, int len)
   {   int rc;
       ULONG ulBytesDone;
     if(len > 0x8000)
        return SendLongDataToServer(data, len);

       rc = DosWrite(Hpipe,data,len, &ulBytesDone);
      if(ulBytesDone != len && rc == 0)
         rc = -1;
       return rc;
   }

   int SendLongDataToServer(void *data, int len)
   {  int i, nraz,rc=0, portionLen, lenSend,rc0;
      char *portionData;
      nraz = (len + 0x7fff) / 0x8000;
      lenSend = 0;
      for(i=0;i<nraz;i++)
      {    portionData = ((char *)data) + 0x8000 * i;
           portionLen  = 0x8000;
           if(i == nraz -1)
                    portionLen = len - 0x8000*i;
           rc0 = SendDataToServer(portionData,portionLen);
           lenSend  += portionLen;
           if(rc0 != 0)
           {   rc = -1;
               break;
           }
      }
      return rc;
   }


   int RecvDataFromClient(void *data, int *len, int maxlen)
   { int rc,raz=0;
     int len0;
     char *pdata;
     if(maxlen > 0x8000)
        return RecvLongDataFromClient(data, len, maxlen);

     len0=0;
     *len = 0;
M:   rc =   DosRead(Hpipe, data, maxlen,(PULONG) &len0);
//     if(rc == ERROR_MORE_DATA || (rc == NO_ERROR && len0 < maxlen ) )
//херня какая-то: в районе порядка 32-64K получается, что при передаче сообщений переменной длины
// могут возникнуть проблемы - в rc будет 0, а данные обрежутся. или это бага конкретной версии ver/r ?

     if(rc == ERROR_MORE_DATA )
     {  maxlen -= len0;
        pdata = (char *) data;
        pdata += len0;
        data = (void *) pdata;
        *len += len0;
        if( maxlen <= 0)
                 return rc;
        raz++;
        if(raz < 8 ||  rc == ERROR_MORE_DATA)
                                               goto M;
     }
     if(rc == NO_ERROR)
             *len += len0;

     return rc;
   }

/* получить данные более 32K */
   int RecvLongDataFromClient(void *data, int *len, int maxlen)
   {  int i, nraz,rc=0, portionLen, len0,rc0;
      char *portionData;
      nraz = (maxlen + 0x7fff) / 0x8000;
      *len = 0;
      for(i=0;i<nraz;i++)
      {    portionData = ((char *)data) + 0x8000 * i;
           portionLen  = 0x8000;
           if(i == nraz -1)
                    portionLen = maxlen - 0x8000*i;
           rc0 = RecvDataFromClient(portionData, &len0, portionLen);
//     sgLogError("Получен от клиента %i кускок %i байт ( portionLen=%i) rс=%x ", i, len0,  portionLen,rc0);
           *len += len0;
           if( (rc0 != ERROR_MORE_DATA) && (rc0 != NO_ERROR) )
           {  rc = rc0;
              break;
           }
      }
//     sgLogError("Получено от клиента %i байт %i кусками ", *len, nraz);
      return rc;
   }


   int RecvCmdFromClient(int *ncmd, int *data)
   {  int rc, *pIntData,maxlen,len;
      ULONG ulBytesDone;
      char str[32], *pdata;

      pdata = &str[0];
      maxlen = sizeof(int) * 2;
      len = 0;

M:    rc =  DosRead(Hpipe, (void *)pdata, maxlen,&ulBytesDone);
      if(rc == ERROR_MORE_DATA)
      {  maxlen -= ulBytesDone;
         pdata += ulBytesDone;
         len += ulBytesDone;
         if( maxlen > 0)
                          goto M;
/******* 26.02.2002 ********/
        if(len == sizeof(int) * 2)
        {   rc = NO_ERROR;
            ulBytesDone = 0;
        }
/******* 26.02.2002 ********/
      }
      if(rc == NO_ERROR)
             len += ulBytesDone;
      if(rc == ERROR_BROKEN_PIPE)
      {  len = 0;
      }
     pIntData = (int *)&str[0];
     *ncmd =  *pIntData;
     *data =  pIntData[1];
      if(len != sizeof(int) * 2)
      {   if(rc == 0)
          {   if(len == 0) *ncmd = 0; //pipe is closed
              else
              {  //  printf("Фигня: RecvCmdFromClient Get %i bytes, cmd=%x, data=[%x]\n",
                 //                                len,ncmd,data);
                   rc = -1;
              }
          }
      }
     return rc;
   }

   int QueryState(void)
   {  int rc;
      ULONG state;
      rc = DosQueryNPHState(Hpipe,&state);
      return rc;
   }

};

/*********************************************/
/*********************************************/
int QueryThreadOrdinal(int &tid);

class ThreadPipe:public NPipe
{
public:
    int used;
    int threadOrdinal;
    int threadTid;
    char buf[MAX_PIPE_BUF];
    ThreadPipe(void):NPipe(buf,CLIENT_MODE)
    {  used = 0;
      threadOrdinal = 0;
      threadTid=0;
    }
    ~ThreadPipe()
    { if(Hpipe)
      {  DosClose(Hpipe);
      }
      Hpipe = NULL;
      used = 0;
    }


    int InitClientConnection(char *externMachineName);

};

class ProccessPipes
{
public:
  class  ThreadPipe pipe[32]; /* для каждой нитки - свой пайп */
  int LSNpipes;             /* число используемых пайпов    */
  char *externMachineName;
static volatile int Access;
  ProccessPipes(void)
  {  LSNpipes = 32;
     externMachineName = NULL;
  }
/* запрос индекса пайпа для текущей нитки */
  int QueryThreadPipeNum(void)
  {  int i,ord,tid;
     ord = QueryThreadOrdinal(tid);
     for(i=0;i<LSNpipes;i++)
     {   if(pipe[i].used && pipe[i].threadOrdinal == ord
                         && pipe[i].threadTid == tid)
          return i;
     }
     return -1;
  }

  ThreadPipe *ThreadPipeOpen(int &ierr )
  {  int i,np,n,rc,tid;
     class  ThreadPipe *pPipe;
     int rcS,raz=0;
     np = QueryThreadPipeNum();
     if(np >= 0)
     {  pPipe = &pipe[np];
        ierr = 0;
        return pPipe;
     }
     n = -1;

     do
     {  rcS =  __lxchg(&Access,LOCKED);

        if(rcS)
        { if(++raz  < 3)  DosSleep(0);
          else            DosSleep(1);
        }
      } while(rcS);     // доступ закрыт

     for(i=0;i<LSNpipes;i++)
     {   if(!pipe[i].used)
         {  n = i;
            pipe[n].used = 1; /* надо будет засемафорить */
            break;
         }
     }
     __lxchg(&Access,UNLOCKED);

     if(n == -1)
     {      ierr = -2;
            return NULL; /* нет больше пайпов */
     }
     rc = pipe[n].InitClientConnection(externMachineName);
     if(rc)
     {  ierr = rc;
        pipe[n].used = 0;
        return NULL;
     }
     pipe[n].used = 1;
     pipe[n].threadOrdinal = QueryThreadOrdinal(tid);
     pipe[n].threadTid = tid;
     pPipe = &pipe[n];
     ierr = 0;
     return pPipe ;
  }

  int CloseThread(void)
  {  int np;
     np = QueryThreadPipeNum();
     if(np < 0)
           return 1;
     if(pipe[np].Hpipe)
     {  DosClose(pipe[np].Hpipe);
        pipe[np].Hpipe = NULL;
     }
     pipe[np].used = 0;
     pipe[np].threadOrdinal = 0;
     pipe[np].threadTid     = 0;
     return 0;
  }
};

#endif
    /* FREEPM_PIPES */
