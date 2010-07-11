/*
 $Id: Fs_hab.hpp,v 1.1 2002/09/17 17:45:23 evgen2 Exp $
*/
/* Fs_hab.hpp */
/* functions for HAB management */
/* ver 0.00 25.08.2002       */

#ifndef FREEPMS_HAB
  #define FREEPMS_HAB

/*************************************************/
/* Server side */
struct _FreePM_HABserverinf
{
   int nsrvthread;     /* server Thread index   */
   int used;           /* =1 - used */
   int clientId;       /* magic number, assigned to client */
                       /* may be usefull for situations with client reconnect  */
   time_t disconnect;  /* time of client disconnect, without F_CMD_CLIENT_EXIT */
};

/* поддерживаемый и раздаваемый сервером список  (i)HAB'ов */
class _FreePM_HAB_serverlist
{  int n;       /* number of HABinf */
   int nAlloced;/* alloceted length of pHabSvrInf */
   struct _FreePM_HABserverinf *pHabSvrInf;
   volatile int Access;      /* fast access semaphor  */
public:
   Fs_Queue Queue;  /* server master queue */
public:
   _FreePM_HAB_serverlist(void)
   { n = nAlloced = 0;
     pHabSvrInf = NULL;
   }
   ~_FreePM_HAB_serverlist(void)
   {
      if(pHabSvrInf)
      {  free(pHabSvrInf);
         pHabSvrInf = NULL;
         n = nAlloced = 0;
      }
   }
   int Add(int &iClientId, int threadNum);
   int Free(int iHab);
   int GetFree(void);
   int GetFreeOldest(double &dtmax);
};

#endif
   /*  FREEPMS_HAB  */
