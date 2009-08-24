/*    
	WinSock.c	1.20
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include "windows.h"
#include "winsock.h"
#include "TwinSock.h"
#include "Driver.h"

HINSTANCE TWIN_GetInstanceFromTask(HTASK hTask);

static	int	wsarc = 0;
static	int	WinSock_Handle = 0;

static struct per_socket_info *TWIN_Init_Socket_Info ( UINT);
static struct per_socket_info *GetPerSockInfo ( SOCKET );
static struct per_task_info *GetTaskInfo ( HTASK );
static struct per_socket_info *CreateNewSocket ( struct per_task_info *, SOCKET);
static BOOL BlockingHook ( struct per_task_info *ppt );
/* WINSOCK API Calls */

SOCKET PASCAL FAR
Accept ( SOCKET sock, struct sockaddr FAR *addr, int FAR *addrlen)
{
  SOCKET    Src = 0;
  struct per_task_info *ppt;
  struct per_socket_info *pps, *ppsNew;

   ppt = (struct per_task_info*)GetTaskInfo(GetCurrentTask());
   pps = (struct per_socket_info *) GetPerSockInfo((SOCKET)sock);

   for (;;) {
      if (ppt->bCancel==TRUE) break;
       Src = (SOCKET) DRVCALL_WINSOCK(PWS_ACCEPT, (SOCKET)sock,(int *) addrlen, (struct sockaddr *)addr); 
      if ((int)Src == -1 ) ppt->bBlocking = TRUE;
      if ((int)Src != -1 ) break;
      while (BlockingHook(ppt))
         ;
      }
   ppt->bBlocking = FALSE;
   ppsNew = (struct per_socket_info*)CreateNewSocket(ppt,Src);
   ppsNew->iFlags |= TWIN_SOCKET_STATE_CONNECT;
   if (pps->iSocketState & TWIN_SOCKET_MASTER ){
      ppsNew->iSocketState |= TWIN_SOCKET_SLAVE;
      ppsNew->iEvents =  pps->iEvents;
      ppsNew->wMsg_from_parent = pps->wMsg_from_parent;
      ppsNew->hParentWnd = pps->hParentWnd;
      ppsNew->iWSACalled = pps->iWSACalled;
      ppsNew->ParentSock = pps->sock;
      if ( (ppsNew->iEvents & FD_READ) || (ppsNew->iEvents & FD_WRITE)) 
      PostMessage(hChildSock,TWIN_ASYNCSELECT,(SOCKET)Src,(long)TWIN_CHECKDATAPENDING) ;
   }
   return (SOCKET) Src;
}
/* Bind */

int PASCAL FAR Bind (SOCKET sock,const struct sockaddr FAR * addr_sock, int Sock_namelen)
{
        return (int) DRVCALL_WINSOCK(PWS_BIND,(SOCKET)sock,(int)Sock_namelen,(const struct sockaddr *)addr_sock);
}

/* CloseSocket */
int PASCAL FAR
Closesocket (SOCKET sock)
{
      int rc = (int) DRVCALL_WINSOCK(PWS_CLOSESOCKET,(SOCKET) sock, 0, 0);
      return rc;
}

/* Connect */
int PASCAL FAR
Connect (SOCKET sock, const struct sockaddr FAR *Sock_name, int Sock_namelen)
{
 struct per_task_info *ppt;
 struct per_socket_info *pps;
 int rc;
   ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
   pps = (struct per_socket_info*)GetPerSockInfo(sock);
   rc = (int) DRVCALL_WINSOCK(PWS_CONNECT,(SOCKET)sock,(int)Sock_namelen,(const struct sockaddr *)Sock_name);
   if ( (int) rc == -1  ) {
        wsarc = WSAGetLastError();
   } 
   if ( rc != -1 ) {
        pps->iFlags |= TWIN_SOCKET_STATE_CONNECT; 
   }
   return rc;
}

/* Ioctlsocket */
int PASCAL FAR Ioctlsocket (SOCKET s, long cmd, DWORD  FAR *argp)
{
   if ( (int)DRVCALL_WINSOCK(PWS_IOCTLSOCKET,(SOCKET)s,(long)cmd,(DWORD FAR *)argp) < 0 ) {
        wsarc = WSAGetLastError();
        return -1;
     }
   return 0; 
}
/* Getpeername */
int PASCAL FAR Getpeername (SOCKET sock, struct sockaddr FAR *name,
                            int FAR * Sock_namelen)
{
        return((int) DRVCALL_WINSOCK(PWS_GETPEERNAME,(SOCKET)sock,(int *)Sock_namelen,(struct sockaddr *)name));

}

int PASCAL FAR Getsockname (SOCKET sock, struct sockaddr FAR *name,
                            int FAR * Sock_namelen)
{
        return((int) DRVCALL_WINSOCK(PWS_GETSOCKNAME,(SOCKET)sock,(int *)Sock_namelen,(struct sockaddr *)name));
}

int PASCAL FAR Getsockopt (SOCKET s, int level, int optname,
                           char FAR * optval, int FAR *optlen)
{
   struct get_db_info *dbinfo;
   int rc;

   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = optval ;
   dbinfo->db_arg2 = optname;
   dbinfo->db_arg3 = level;
   rc = (int) DRVCALL_WINSOCK(PWS_GETSOCKOPT,(SOCKET)s,(int*)optlen,(struct get_db_info*)dbinfo);
   if ( rc < 0 ) wsarc = WSAGetLastError();
   WinFree(dbinfo);
   return rc;

}

DWORD  PASCAL FAR Htonl (DWORD  hostlong)
{
    return((DWORD)DRVCALL_WINSOCK(PWS_HTONL,(DWORD)hostlong,0,0));
}

WORD    PASCAL FAR Htons (WORD    hostshort)
{
    return((DWORD)DRVCALL_WINSOCK(PWS_HTONS,(WORD)hostshort,0,0));
}

unsigned long PASCAL FAR Inet_addr (const char FAR * cp)
{
   return (DWORD)DRVCALL_WINSOCK(PWS_INET_ADDR,(const char FAR *)cp,0,0);
}
/* RD for now. */ 
char FAR * PASCAL FAR Inet_ntoa (struct in_addr in)
{
     static char *asci_net;
     asci_net = (char *)DRVCALL_WINSOCK(PWS_INET_NTOA,asci_net,0,&in);
     return(char * )asci_net;
}

int PASCAL FAR Listen (SOCKET sock , int backlog)
{
    struct per_socket_info *pps;
    struct per_task_info *ppt;
    int rc;

    if ((ppt = (struct per_task_info*)GetTaskInfo(GetCurrentTask())) == 0 )
      return -1;
    if ( (pps = (struct per_socket_info*)GetPerSockInfo((SOCKET) sock)) == 0 ) 
      return -1;
    if (!pps->iSocketState & TWIN_SOCKET_MASTER ) return -1;
    rc = (int)DRVCALL_WINSOCK(PWS_LISTEN,(SOCKET)sock,(int)backlog,0);
               if ( (int) rc == -1  ) {
                 wsarc = WSAGetLastError();
               } 
    if ( rc != -1 ) {
       pps->iFlags |= TWIN_SOCKET_STATE_ACCEPT; 
    }
    return (rc);    
     
}

DWORD  PASCAL FAR Ntohl (DWORD  netlong)
{
	return((DWORD)DRVCALL_WINSOCK(PWS_NTOHL,(DWORD) netlong,0,0));
}

WORD    PASCAL FAR Ntohs (WORD    netshort)
{
	return((WORD)DRVCALL_WINSOCK(PWS_NTOHS,(WORD) netshort,0,0));
}

int PASCAL FAR Recv (SOCKET s, char FAR * buf, int len, int flags)
{

   return(int) Recvfrom( s, buf, len, flags,0,0);
}

int PASCAL FAR Recvfrom (SOCKET s, char FAR * buf, int len, int flags,
                         struct sockaddr FAR *from, int FAR * fromlen)
{
   struct per_task_info *ppt;
   struct per_socket_info *pps;
   struct sock_data *RecvParam;
   int rc = 0;

   ppt = (struct per_task_info*)GetTaskInfo(GetCurrentTask());
   pps = (struct per_socket_info*)GetPerSockInfo((SOCKET)s);
   RecvParam = (struct sock_data*) WinMalloc ( sizeof(struct sock_data)); 
   RecvParam->sock = (SOCKET)s;
   RecvParam->SockData = (char *)buf;
   RecvParam->DataLen      = (int)len;
   RecvParam->DataFlags   = flags;
   RecvParam->from_addr = (struct sockaddr *) from;
   RecvParam->from_len  = (int *) fromlen;
   for (;;) {
	if (ppt->bCancel == TRUE)
	    break;
	rc = (int)DRVCALL_WINSOCK(PWS_RECVFROM,(SOCKET)s,(struct sock_data*)RecvParam,0);
	if (rc == -1 )
	    ppt->bBlocking = TRUE;
	else
	    break;

	while (BlockingHook(ppt));
    } /* endfor() */

    ppt->bBlocking = FALSE;
    if ((int)rc == 0 && pps->iWSACalled == TRUE ) {
     PostMessage(pps->hParentWnd,pps->wMsg_from_parent,(SOCKET)s,WSAMAKESELECTREPLY(FD_CLOSE,0));  
     DeleteSocket (pps);
    }
   WinFree(RecvParam);
     return (int) rc;
}

int PASCAL FAR Select (int nfds ,
		 fd_set FAR *readfds, fd_set FAR *writefds,
                 fd_set FAR *exceptfds, const struct timeval FAR *timeout) 
{
 struct per_task_info *ppt;
 int i;
 unsigned long	TimedOut=0;
 struct timeval SelectTime; 
 struct SelectInfo *Select_info;
 struct SelectInfo *Select_info_sav;

 ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
 if (timeout)
   {
     TimedOut = GetTickCount();
     TimedOut += timeout->tv_usec /1000 + timeout->tv_sec * 1000;
   } 
  Select_info = (struct SelectInfo *)WinMalloc(sizeof(struct SelectInfo));
  Select_info_sav = (struct SelectInfo *)WinMalloc(sizeof(struct SelectInfo));
  Select_info->read_fds_info = (fd_set *)WinMalloc(sizeof(fd_set));
  Select_info->write_fds_info = (fd_set *)WinMalloc(sizeof(fd_set));
  Select_info->except_fds_info = (fd_set *)WinMalloc(sizeof(fd_set));
  Select_info_sav->read_fds_info = (fd_set *)WinMalloc(sizeof(fd_set));
  Select_info_sav->write_fds_info = (fd_set *)WinMalloc(sizeof(fd_set));
  Select_info_sav->except_fds_info = (fd_set *)WinMalloc(sizeof(fd_set));
  	Select_info->read_fds_info = (fd_set *)readfds->fds_bits;
  	Select_info->write_fds_info= (fd_set*)writefds->fds_bits;
  	Select_info->except_fds_info= (fd_set*)exceptfds->fds_bits;
        for ( i=0; i < 8 ; i++) 
        Select_info_sav->read_fds_info->fds_bits[i] = Select_info->read_fds_info->fds_bits[i];
        for ( i=0; i< 8  ; i++) 
        Select_info_sav->write_fds_info->fds_bits[i] = Select_info->write_fds_info->fds_bits[i];
        for ( i=0; i< 8 ; i++) 
        Select_info_sav->except_fds_info->fds_bits[i] = Select_info->except_fds_info->fds_bits[i];
 	SelectTime.tv_sec = 0;
  	SelectTime.tv_usec = 0;
  	Select_info->Select_Time_Value = &SelectTime; 
  if ((timeout->tv_sec == 0) && (timeout->tv_usec == 0) ) {
        nfds = (int) DRVCALL_WINSOCK(PWS_SELECT,0,0,(struct SelectInfo *)Select_info);
      }
  else 
     { 
       for (;;) {
        for ( i=0; i< 8; i++) 
        Select_info->read_fds_info->fds_bits[i] = Select_info_sav->read_fds_info->fds_bits[i];
        for ( i=0; i< 8; i++) 
        Select_info->write_fds_info->fds_bits[i] = Select_info_sav->write_fds_info->fds_bits[i];
        for ( i=0; i< 8; i++) 
        Select_info->except_fds_info->fds_bits[i] = Select_info_sav->except_fds_info->fds_bits[i];
        if (ppt->bCancel==TRUE) break;
        nfds = (int) DRVCALL_WINSOCK(PWS_SELECT,0,0,(struct SelectInfo *)Select_info);
        if (nfds == -1 ) break; 
        if (nfds == 0 ) ppt->bBlocking = TRUE; 
        if ( (nfds > 0) || (timeout && GetTickCount() >= TimedOut) ) break;  
        while (BlockingHook(ppt))
           ;
       }
  } 
  readfds = Select_info->read_fds_info;
  writefds = Select_info->write_fds_info;
  exceptfds = Select_info->except_fds_info; 
  timeout = Select_info->Select_Time_Value;
  WinFree(Select_info);
  WinFree(Select_info_sav);
  return (int)nfds;
}
int PASCAL FAR Send (SOCKET s, const char FAR * buf, int len, int flags)
{
     return (int) Sendto ( s, buf, len, flags, 0, 0 );
}

int PASCAL FAR Sendto (SOCKET s, const char FAR * buf, int len, int flags,
                       const struct sockaddr FAR *to, int tolen)
{

   struct per_socket_info *pps;
   struct sock_data *SendParam;
   int rc;

   pps = (struct per_socket_info*)GetPerSockInfo((SOCKET)s);
   SendParam = (struct sock_data*) WinMalloc ( sizeof(struct sock_data)); 
   SendParam->sock = (SOCKET)s;
   SendParam->SockData = (char *)buf;
   SendParam->DataLen      = (int)len;
   SendParam->DataFlags   = flags;
   SendParam->from_addr = (struct sockaddr *) to;
   SendParam->to_len  = (int ) tolen;
   rc = (int) DRVCALL_WINSOCK(PWS_SENDTO,(SOCKET)s,(struct sock_data*)SendParam,0);
   return rc;
}

int PASCAL FAR Setsockopt (SOCKET s, int level, int optname,
                           const char FAR * optval, int optlen)
{
   struct get_db_info *dbinfo;
   int rc;

   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = optval ;
   dbinfo->db_arg2 = optname;
   dbinfo->db_arg3 = optlen;
   rc = (int) DRVCALL_WINSOCK(PWS_SETSOCKOPT,(SOCKET)s,(int)level,(struct get_db_info*)dbinfo);
   if ( rc < 0 ) wsarc = WSAGetLastError();
   WinFree(dbinfo);
   return rc;
}

int PASCAL FAR Shutdown (SOCKET s, int how)
{
   int rc = (int) DRVCALL_WINSOCK(PWS_SHUTDOWN,(SOCKET)s,(int)how,0);
   if ( rc < 0 )
     {
        wsarc = WSAGetLastError();
     }
   return rc;    
}

SOCKET PASCAL FAR Socket (int af, int type, int protocol)
{
     UINT New_Sock;
     struct per_socket_info *pps; 
     New_Sock = (UINT) DRVCALL_WINSOCK(PWS_SOCKET,af,type,protocol);
     if ((int) New_Sock == -1 ) {
         wsarc = WSAGetLastError();
         return (New_Sock);
     }   
     pps = TWIN_Init_Socket_Info(New_Sock);
     pps->iSocketState |= TWIN_SOCKET_MASTER;
     pps->ParentSock = 0;
     return(New_Sock);
}
/* Database function prototypes */

struct hostent FAR * PASCAL FAR Gethostbyaddr(const char FAR * addr,
                                              int len, int type)
{

   struct get_db_info *dbinfo;
   HOSTENT *hst;
   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = addr ;
   dbinfo->db_char_arg2 = NULL;
   dbinfo->db_arg2 = len;
   dbinfo->db_arg3 = type;
   hst = (HOSTENT*)DRVCALL_WINSOCK(PWS_GETHOSTBYADDR,0,(struct get_db_info*)dbinfo,0);
   WinFree (dbinfo); 
   return(HOSTENT*)hst;
}

struct hostent FAR * PASCAL FAR Gethostbyname(const char FAR * name)
{
   struct get_db_info *dbinfo;
   HOSTENT *hst;
   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = name ;
   dbinfo->db_char_arg2 = NULL;
   dbinfo->db_arg2 = 0;
   dbinfo->db_arg3 = 0;
   hst = (HOSTENT*)DRVCALL_WINSOCK(PWS_GETHOSTBYNAME,0,(struct get_db_info*)dbinfo,0);
   WinFree (dbinfo); 
   return(HOSTENT*)hst;
}

int PASCAL FAR Gethostname (char FAR * name, int namelen)
{
    return (int) DRVCALL_WINSOCK(PWS_GETHOSTNAME,(char *)name,(int)namelen,0);
}

struct servent FAR * PASCAL FAR Getservbyport(int port, const char FAR * proto)
{
   struct get_db_info *dbinfo;
   SERVENT *srv;
   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = NULL ;
   dbinfo->db_char_arg2 = proto;
   dbinfo->db_arg2 = port;
   dbinfo->db_arg3 = 0;
   srv = (SERVENT*) DRVCALL_WINSOCK(PWS_GETSERVBYPORT,0,(struct get_db_info*)dbinfo,0);
   WinFree (dbinfo);
   return(srv);
}

struct servent FAR * PASCAL FAR Getservbyname(const char FAR * name,
                                              const char FAR * proto)
{
   struct get_db_info  *dbinfo;
   SERVENT *srv;
   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = name;
   dbinfo->db_char_arg2 = proto;
   dbinfo->db_arg2 = 0;
   dbinfo->db_arg3 = 0;
   srv =(SERVENT*) DRVCALL_WINSOCK(PWS_GETSERVBYNAME,0,(struct get_db_info*)dbinfo,0);
   WinFree(dbinfo);
   return(srv);    
}

struct protoent FAR * PASCAL FAR Getprotobynumber(int proto)
{
   struct get_db_info  *dbinfo;
   PROTOENT *proent;
   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = NULL;
   dbinfo->db_char_arg2 = NULL;
   dbinfo->db_arg2 = proto;
   dbinfo->db_arg3 = 0;
   proent =(PROTOENT*) DRVCALL_WINSOCK(PWS_GETPROTOBYNUMBER,0,(struct get_db_info*)dbinfo,0);
   WinFree(dbinfo);
   return(proent);    

}

struct protoent FAR * PASCAL FAR Getprotobyname(const char FAR * name)
{

   struct get_db_info  *dbinfo;
   PROTOENT *proent;
   dbinfo = (struct get_db_info*)WinMalloc(sizeof(struct get_db_info)); 
   dbinfo->db_char_arg1 = name;
   dbinfo->db_char_arg2 = NULL;
   dbinfo->db_arg2 = 0;
   dbinfo->db_arg3 = 0;
   proent =(PROTOENT*) DRVCALL_WINSOCK(PWS_GETPROTOBYNAME,0,(struct get_db_info*)dbinfo,0);
   WinFree(dbinfo);
   return(proent);    
}
/* Microsoft Windows Extension function prototypes */

int PASCAL FAR WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
        struct per_task_info *pptNew;
/* Make sure that the version requested is >= 1.1      */
/* The low byte is the major version and the high byte */
/* is the minor version				       */
       
 	if ( LOBYTE (wVersionRequired  ) < 1 ||
		( LOBYTE ( wVersionRequired  ) == 1 &&
		  HIBYTE ( wVersionRequired  ) < 1 ) )
	{
		return -1; /*WSAVERNOTSUPPORTED; */
	}
/* Since we support only version 1.1, set both the wVersion and  */
/* whighVersion to 1.1                                           */        
        lpWSAData->wVersion = MAKEWORD(1,1); /* 0x0101; */
	lpWSAData->wHighVersion = MAKEWORD(1,1); /* 0x0101;*/ 
/* The initialization has been done here so we fill the remaining */
/* elements of the lpWSAData structure                            */
	strcpy(lpWSAData->szDescription,
		"Twin Winsock Implementation 1.0\n"
		"Copyright Willows Software Inc.\n"
		"Aug 10 1995 Revision 0.0       \n");
	if ( GetCurrentTask() != '\0' ) strcpy(lpWSAData->szSystemStatus,"Ready");
	else {
               strcpy(lpWSAData->szSystemStatus,"Not Initialized");
               return -1; /* WSASYSNOTREADY; */
        }
	lpWSAData->iMaxSockets = 256;
	lpWSAData->iMaxUdpDg = 1024;
	lpWSAData->lpVendorInfo = 0;

	pptNew = (struct per_task_info *)
		WinMalloc (sizeof(struct per_task_info));
        memset((LPSTR)pptNew,0,sizeof(struct per_task_info)); 
        pptNew->htask = GetCurrentTask();
	pptNew->pptNext = pptList;
	pptNew->lpBlockFunc = 0;
	pptNew->bCancel = FALSE;
	pptNew->bBlocking = FALSE;
	pptList = pptNew;
        if(!InitTwinWinsock(pptNew->htask)) return -1;
        return 0;
}

int PASCAL FAR WSACleanup(void)
{
 	struct per_task_info *ppt;
        HTASK	htask;
	
	htask = GetCurrentTask();
        for ( ppt = pptList ; ppt ; ppt = ppt->pptNext )
            if ( ppt->htask == htask ) {
      		/* Check for Blocking here before closing up */
		   DeleteSocketTask( ppt );
                   break;
            } 
        return 0;
}

void PASCAL FAR WSASetLastError(int iError)
{
     DRVCALL_WINSOCK(PWS_WSASETLASTERROR,iError,0,0);
     return;
}

int PASCAL FAR WSAGetLastError(void)
{
	return (int) DRVCALL_WINSOCK(PWS_WSAGETLASTERROR,0,0,0);
}

BOOL PASCAL FAR WSAIsBlocking(void)
{
  struct per_task_info *ppt;
  
  if ((ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask())) == 0)
           return -1;
  return (ppt->bBlocking);

}

int PASCAL FAR WSAUnhookBlockingHook(void)
{
  struct per_task_info *ppt;
 
  if ((ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask())) == 0)
          return -1;
  ppt->lpBlockFunc = 0;
  return 0;
}

FARPROC PASCAL FAR WSASetBlockingHook(FARPROC lpBlockFunc)
{
  struct per_task_info *ppt;
  if ((ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask())) == 0)
     return 0;
  ppt->lpBlockFunc = lpBlockFunc;
  return 0; /* Check the return value later */
}

int PASCAL FAR WSACancelBlockingCall(void)
{
   struct per_task_info *ppt;
   ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
   if (ppt->bBlocking) 
     {
        ppt->bCancel = TRUE;
        return(0);
     }
   else return (-1);
}

HANDLE PASCAL FAR WSAAsyncGetServByName(HWND hWnd, UINT  wMsg,
                                        const char FAR *name, 
                                        const char FAR *proto,
                                        char FAR *buf, int buflen)
{
  struct per_task_info *ppt;
  HANDLE WSA_RetHandle;
	WSA_RetHandle = ReturnWSAHandle();
   	ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
        if (ppt == 0) return 0;
        if (!proto) proto = "";
        ppt->AsyncWndHandle = hWnd;
        ppt->AsyncWndMsg = wMsg;
        ppt->pAsyncBuffer = buf;
        ppt->iAsyncLen    = buflen;
	if ( ppt->name )
		WinFree(ppt->name);
        ppt->name = (LPSTR) WinStrdup(name);
	if ( ppt->proto )
		WinFree(ppt->proto);
        ppt->proto = (LPSTR) WinStrdup(proto);
	PostMessage(hChildSock,TWIN_ASYNCGETSERVBYNAME,0,0);
	return (WSA_RetHandle);

}

HANDLE PASCAL FAR WSAAsyncGetServByPort(HWND hWnd, UINT  wMsg, int port,
                                        const char FAR *proto, char FAR *buf,
                                        int buflen)
{
  struct per_task_info *ppt;
  HANDLE WSA_RetHandle;
	WSA_RetHandle = ReturnWSAHandle();
   	ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
        if (ppt == 0) return 0;
        if (!proto) proto = "";
        ppt->AsyncWndHandle = hWnd;
        ppt->AsyncWndMsg = wMsg;
        ppt->pAsyncBuffer = buf;
        ppt->iAsyncLen    = buflen;
	if ( ppt->proto )
		WinFree(ppt->proto);
        ppt->proto = (LPSTR) WinStrdup(proto);
	PostMessage(hChildSock,TWIN_ASYNCGETSERVBYPORT,0,(int)port);

	return (WSA_RetHandle);

}

HANDLE PASCAL FAR WSAAsyncGetProtoByName(HWND hWnd, UINT  wMsg,
                                         const char FAR *name, char FAR *buf,
                                         int buflen)
{
  struct per_task_info *ppt;
  HANDLE WSA_RetHandle;
	WSA_RetHandle = ReturnWSAHandle();
   	ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
        if (ppt == 0) return 0;
        ppt->AsyncWndHandle = hWnd;
        ppt->AsyncWndMsg = wMsg;
        ppt->pAsyncBuffer = buf;
        ppt->iAsyncLen    = buflen;
	if ( ppt->name )
		WinFree(ppt->name);
        ppt->name = (LPSTR) WinStrdup(name);
	PostMessage(hChildSock,TWIN_ASYNCGETPROTOBYNAME,0,0);
	return (WSA_RetHandle);

}

HANDLE PASCAL FAR WSAAsyncGetProtoByNumber(HWND hWnd, UINT  wMsg,
                                           int number, char FAR *buf,
                                           int buflen)
{
  struct per_task_info *ppt;
  HANDLE WSA_RetHandle;
	WSA_RetHandle = ReturnWSAHandle();
   	ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
        if (ppt == 0) return 0;
        ppt->AsyncWndHandle = hWnd;
        ppt->AsyncWndMsg = wMsg;
        ppt->pAsyncBuffer = buf;
        ppt->iAsyncLen    = buflen;
	PostMessage(hChildSock,TWIN_ASYNCGETPROTOBYNUMBER,(int)number,0);
	return (WSA_RetHandle);

}

HANDLE PASCAL FAR WSAAsyncGetHostByName(HWND hWnd, UINT  wMsg,
                                        const char FAR *name, char FAR *buf,
                                        int buflen)
{
  struct per_task_info *ppt;
  HANDLE WSA_RetHandle;
	WSA_RetHandle = ReturnWSAHandle();
   	ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
        if (ppt == 0) return 0;
        ppt->AsyncWndHandle = hWnd;
        ppt->AsyncWndMsg = wMsg;
        ppt->pAsyncBuffer = buf;
        ppt->iAsyncLen    = buflen;
	if ( ppt->name )
		WinFree(ppt->name);
        ppt->name = (LPSTR) WinStrdup(name);
	PostMessage(hChildSock,TWIN_ASYNCGETHOSTBYNAME,0,0);
	return (WSA_RetHandle);

}

HANDLE PASCAL FAR WSAAsyncGetHostByAddr(HWND hWnd, UINT  wMsg,
                                        const char FAR *addr, int len, int type,
                                        char FAR *buf, int buflen)
{
  struct per_task_info *ppt;
  HANDLE WSA_RetHandle;
	WSA_RetHandle = ReturnWSAHandle();
   	ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
        if (ppt == 0) return 0;
        ppt->AsyncWndHandle = hWnd;
        ppt->AsyncWndMsg = wMsg;
        ppt->pAsyncBuffer = buf;
        ppt->iAsyncLen    = buflen;
	if ( ppt->name )
		WinFree(ppt->name);
        ppt->name = (LPSTR) WinStrdup(addr);
	PostMessage(hChildSock,TWIN_ASYNCGETHOSTBYADDR,0,(int)len);
	return (WSA_RetHandle);

}

int PASCAL FAR WSACancelAsyncRequest(HANDLE hAsyncTaskHandle)
{
        return 0;
}

int PASCAL FAR WSAAsyncSelect(SOCKET s, HWND hWnd, UINT  wMsg, long lEvent)
{

        struct per_socket_info *pps;
        if((pps = GetPerSockInfo(s)) == 0) return -1;
        pps->hParentWnd = hWnd;
        pps->wMsg_from_parent = wMsg;
        pps->iEvents = lEvent;
        pps->iWSACalled = TRUE;
        if ( wMsg == 0 && lEvent == 0 )
        return (0);
        else
	PostMessage(hChildSock,TWIN_ASYNCSELECT,(SOCKET)s,(long)lEvent) ;
	return (0);
}

HANDLE PASCAL FAR WSAAsyncGetHostName ( HWND hWnd, UINT wMsg, char FAR *name, int length)
{
  HANDLE WSA_RetHandle;
	WSA_RetHandle = ReturnWSAHandle();
	PostMessage(hChildSock,wMsg,length,(LPARAM)name);
	return (WSA_RetHandle);
}

void DeleteSocketTask ( struct per_task_info *ppt ) 

 {
   struct per_task_info **pppt;

     for ( pppt = &pptList; *pppt ; pppt = & ( (*pppt)->pptNext ))
        {
          if ( *pppt == ppt )
	    {
    		*pppt = ppt->pptNext;
		if ( ppt->name )
			WinFree(ppt->name);
		if ( ppt->proto )
			WinFree(ppt->proto);
                WinFree(ppt);
                break;
            }
        }
 } 

BOOL InitTwinWinsock ( HTASK  CurrentTask )

{
        WNDCLASS    Sockwndclass;
        memset((LPSTR)&Sockwndclass,0,sizeof(WNDCLASS)); 
	Sockwndclass.style	= 0;
	Sockwndclass.lpfnWndProc	= SockProc;
	Sockwndclass.cbClsExtra	= 0;
	Sockwndclass.cbWndExtra	= 0;
	Sockwndclass.hInstance	= TWIN_GetInstanceFromTask(CurrentTask);
        Sockwndclass.hIcon	= 0;
	Sockwndclass.hbrBackground= 0;
        Sockwndclass.hCursor    = 0;
	Sockwndclass.lpszMenuName	= NULL;
	Sockwndclass.lpszClassName	= "WILLOWSOCK";
	
	RegisterClass(&Sockwndclass);
        	 	

        hChildSock = CreateWindow ( "WILLOWSOCK",
			NULL,
			WS_POPUP,
			0,0,
			0,0,
			(HWND)NULL,
			(HMENU)NULL,
			(HINSTANCE)TWIN_GetInstanceFromTask(CurrentTask),
			NULL);		 
	if (!hChildSock) return (FALSE);
	return (TRUE);
}

LRESULT CALLBACK SockProc (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)

{
  int rc=0;
  HWND  hParentWnd;
  static struct per_socket_info *pps;
  static struct per_socket_info *pps_parent;
  struct per_task_info *ppt;
  SOCKET s;
  HOSTENT  *he;
  long   event;
  int    iCode;
       switch ( wMsg )
           {
           case TWIN_GETHOSTNAME:
               rc = (int) DRVCALL_WINSOCK (PWS_GETHOSTNAME, (char *)lParam, (int)wParam,0);
               if ( rc == 0 ) {
		 hParentWnd = GetParent(hWnd);
                 PostMessage(hParentWnd,wMsg,wParam,lParam);
                 return rc;
               } 
               break;
          case TWIN_ACCEPT: 
               pps = GetPerSockInfo ( (SOCKET) wParam ); 
               lParam = (LPARAM) pps->SaveSockAddr; 
	       s = Accept((SOCKET)wParam,(struct sockaddr *)&lParam,&pps->SaveSockLen); 
#ifdef __RD
               s =  (SOCKET) DRVCALL_WINSOCK(PWS_ACCEPT, (SOCKET)wParam,(int*)pps->SaveSockLen,(struct sockaddr *)lParam);  
#endif
               if ( (int) s  < 0 ) {
                 rc = WSAGetLastError();
                } 
                hParentWnd = GetParent(hWnd);
                wMsg = pps->wMsg_from_parent;
                wParam = (SOCKET) s;
 		PostMessage(hParentWnd,wMsg,wParam,WSAMAKEASYNCREPLY(0,rc));
                return rc; 
          case	TWIN_ASYNCSELECT:
                ppt = (struct per_task_info*)GetTaskInfo(GetCurrentTask());
                for (;;) {
                 iCode = 0;
                 if (ppt->bCancel == TRUE ) break;
                 if (lParam == 0 ) break;
                 pps = (struct per_socket_info*)GetPerSockInfo ( (SOCKET) wParam );
                 if(pps ==NULL ) {
                   pps =(struct per_socket_info*) GetPerSockInfo ( (SOCKET) pps_parent->sock); 
                   wParam = pps->sock;
                 }
                 s = pps->sock;
                 event = pps->iEvents; 
                 if ( pps->ParentSock != 0  ) {
                      pps_parent = (struct per_socket_info *) GetPerSockInfo((SOCKET)pps->ParentSock); 
                      rc = (long) DRVCALL_WINSOCK(PWS_WSAASYNCSELECT,(SOCKET)pps_parent->sock,(long)FD_READ,0);
                    if ( rc & FD_READ ) 
                      {
                	hParentWnd = pps_parent->hParentWnd;
                	wMsg = pps_parent->wMsg_from_parent;
                	if (pps_parent->iFlags & TWIN_SOCKET_STATE_ACCEPT) iCode |=  FD_ACCEPT | FD_READ ;
                	if ( pps_parent->iEvents & iCode ) 
                	PostMessage(hParentWnd,wMsg,pps_parent->ParentSock,WSAMAKESELECTREPLY(iCode,0));  
                      }
                 }
                 else     
                 if (pps->iSocketState & TWIN_SOCKET_MASTER )
                  {
                    if ( pps->iEvents & FD_CONNECT ) {
                      if ( pps->iFlags & TWIN_SOCKET_STATE_CONNECT ) {
                        rc = (long) DRVCALL_WINSOCK(PWS_WSAASYNCSELECT,(SOCKET)wParam,(long)TWIN_CHECKDATAPENDING,0);
                      }
                      else {
                        rc = (long) DRVCALL_WINSOCK(PWS_WSAASYNCSELECT,(SOCKET)wParam,(long)FD_WRITE,0);
                      }
                    }
                    else  {
                    rc = (long) DRVCALL_WINSOCK(PWS_WSAASYNCSELECT,(SOCKET)wParam,(long)TWIN_CHECKDATAPENDING,0);
                    }
                    if ( rc & FD_READ ) 
                      {
                	hParentWnd = pps->hParentWnd;
                	wMsg = pps->wMsg_from_parent;
                	if (pps->iFlags & TWIN_SOCKET_STATE_ACCEPT) iCode |=  FD_ACCEPT;
                	if (pps->iFlags & TWIN_SOCKET_STATE_CONNECT) iCode |=  FD_READ;
                	if ( pps->iEvents & iCode ) 
                	PostMessage(hParentWnd,wMsg,wParam,WSAMAKESELECTREPLY(iCode,0));  
                      }
                    if ( rc & FD_WRITE ) 
                      {
                	hParentWnd = pps->hParentWnd;
                	wMsg = pps->wMsg_from_parent;
                	if (pps->iFlags & TWIN_SOCKET_STATE_CONNECT) iCode |=  FD_CONNECT | FD_WRITE;
                	if ( pps->iEvents & iCode ) 
                	PostMessage(hParentWnd,wMsg,wParam,WSAMAKESELECTREPLY(iCode,0));  
                      }
                  }
                 if ((pps->iSocketState & TWIN_SOCKET_SLAVE) )/*&& ( lParam & TWIN_CHECKDATAPENDING))*/
                  {
                    rc = (long) DRVCALL_WINSOCK(PWS_WSAASYNCSELECT,(SOCKET)wParam,(long)TWIN_CHECKDATAPENDING,0);
                    if ( rc & FD_READ ) 
                      {
                	hParentWnd = pps->hParentWnd;
                	wMsg = pps->wMsg_from_parent;
               /* 	if (pps->iFlags & TWIN_SOCKET_STATE_ACCEPT)*/ iCode =  FD_READ;
                	if ( pps->iEvents & iCode ) 
                	PostMessage(hParentWnd,wMsg,wParam,WSAMAKESELECTREPLY(iCode,0));  
                       }
                    if ( rc & FD_WRITE ) 
                      {
                	hParentWnd = pps->hParentWnd;
                	wMsg = pps->wMsg_from_parent;
               /* 	if (pps->iFlags & TWIN_SOCKET_STATE_ACCEPT)*/ iCode =  FD_WRITE;
                	if ( pps->iEvents & iCode ) 
                	PostMessage(hParentWnd,wMsg,wParam,WSAMAKESELECTREPLY(iCode,0));  
                       }
                   }
                while(BlockingHook(ppt))
                     ;
                }
                if (pps) {
                pps->iWSACalled = 0;
                if(pps->iFlags & TWIN_SOCKET_STATE_ACCEPT) Closesocket(wParam);
                }
                return 0;
#ifdef NOTUSED
                rc = (long) DRVCALL_WINSOCK(PWS_WSAASYNCSELECT,(SOCKET)wParam,(long)lParam,0);           
                hParentWnd = pps->hParentWnd;
                wMsg = pps->wMsg_from_parent;
                if (pps->iFlags & TWIN_SOCKET_STATE_ACCEPT) iCode =  FD_ACCEPT;
                if ( pps->iEvents & iCode ) 
                PostMessage(hParentWnd,wMsg,wParam,WSAMAKESELECTREPLY(iCode,0));  
#endif
          case  TWIN_ASYNCGETSERVBYNAME:
                ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
                ppt->pAsyncBuffer = (char *)Getservbyname((const char *)ppt->name,(const char *)ppt->proto);
                if ( ppt->pAsyncBuffer) {
                  memcpy(&ppt->se,ppt->pAsyncBuffer,sizeof(struct servent));
                }
                else
                  wsarc = WSAGetLastError(); 
		PostMessage(ppt->AsyncWndHandle,ppt->AsyncWndMsg,0,
                            WSAMAKEASYNCREPLY(strlen(ppt->pAsyncBuffer),0));
                break;
          case  TWIN_ASYNCGETSERVBYPORT:
                ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
                ppt->pAsyncBuffer = (char *)Getservbyport((int)wParam,(const char *)lParam);
                if ( ppt->pAsyncBuffer) {
                  memcpy(&ppt->se,ppt->pAsyncBuffer,sizeof(struct servent));
                }
                else
                  wsarc = WSAGetLastError(); 
		PostMessage(ppt->AsyncWndHandle,ppt->AsyncWndMsg,0,
                            WSAMAKEASYNCREPLY(strlen(ppt->pAsyncBuffer),0));
                break;
          case  TWIN_ASYNCGETHOSTBYNAME:
                ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
                 he  = Gethostbyname(ppt->name);
                if ( he ) {
                  memcpy(ppt->pAsyncBuffer,he,sizeof(struct hostent));
                  memcpy(&ppt->he,ppt->pAsyncBuffer,sizeof(struct hostent));
                }
                else
                  wsarc = WSAGetLastError(); 
		PostMessage(ppt->AsyncWndHandle,ppt->AsyncWndMsg,0,
                            WSAMAKEASYNCREPLY(strlen(ppt->pAsyncBuffer),0));
                break;
          case  TWIN_ASYNCGETHOSTBYADDR:
                ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
                ppt->pAsyncBuffer = (char *)Gethostbyaddr((const char *)ppt->name,(int)lParam,PF_INET);
                if ( ppt->pAsyncBuffer) {
                  memcpy(&ppt->he,ppt->pAsyncBuffer,sizeof(struct hostent));
                }
                else
                  wsarc = WSAGetLastError(); 
		PostMessage(ppt->AsyncWndHandle,ppt->AsyncWndMsg,0,
                            WSAMAKEASYNCREPLY(strlen(ppt->pAsyncBuffer),0));
                break;
          case  TWIN_ASYNCGETPROTOBYNAME:
                ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
                ppt->pAsyncBuffer = (char *)Getprotobyname((const char *)ppt->name);
                if ( ppt->pAsyncBuffer) {
                  memcpy(&ppt->pe,ppt->pAsyncBuffer,sizeof(struct protoent));
                }
                else
                  wsarc = WSAGetLastError(); 
		PostMessage(ppt->AsyncWndHandle,ppt->AsyncWndMsg,0,
                            WSAMAKEASYNCREPLY(strlen(ppt->pAsyncBuffer),0));
                break;
          case  TWIN_ASYNCGETPROTOBYNUMBER:
                ppt = (struct per_task_info *)GetTaskInfo(GetCurrentTask());
                ppt->pAsyncBuffer = (char *)Getprotobynumber((int)wParam);
                if ( ppt->pAsyncBuffer) {
                  memcpy(&ppt->pe,ppt->pAsyncBuffer,sizeof(struct protoent));
                }
                else
                  wsarc = WSAGetLastError(); 
		PostMessage(ppt->AsyncWndHandle,ppt->AsyncWndMsg,0,
                            WSAMAKEASYNCREPLY(strlen(ppt->pAsyncBuffer),0));
                break;
          case  TWIN_ASYNCCANCEL:
                 pps = (struct per_socket_info*)GetPerSockInfo ( (SOCKET) wParam );
                 pps->iWSACalled = 0;
                 if(pps->iFlags & TWIN_SOCKET_STATE_ACCEPT) Closesocket(wParam);
                 return 0;  
           default:
		return(DefWindowProc(hWnd,wMsg, wParam, lParam ));
           }

    return 0;

}

static HANDLE ReturnWSAHandle (void)
 {

  return WinSock_Handle++;

 }

HANDLE PASCAL FAR WSAAsyncAccept ( HWND hWnd ,UINT wMsg, SOCKET sock, struct sockaddr FAR *addr, int FAR *addrlen)
{
        struct per_socket_info *pps;
        pps = GetPerSockInfo(sock);
        pps->hParentWnd = hWnd;
        pps->wMsg_from_parent = wMsg;
        pps->SaveSockAddr = (struct sockaddr *)addr; 
        pps->SaveSockLen  = *addrlen; 
	PostMessage(hChildSock,TWIN_ACCEPT,(SOCKET)sock,(LPARAM)addrlen );
	return (ReturnWSAHandle());
}

int __WSAFDIsSet ( SOCKET s , fd_set FAR * set )

 {
      return (int)DRVCALL_WINSOCK(PWS___WSAFDISSET,(SOCKET)s,0,(fd_set FAR *)set); 

 }

static struct per_socket_info * TWIN_Init_Socket_Info ( UINT sock )

  {

   struct per_socket_info *ppsNew;
   
   ppsNew = (struct per_socket_info *)
		WinMalloc(sizeof(struct per_socket_info)); 
   memset(ppsNew,0,sizeof(struct per_socket_info));
   if ( !ppsNew ) return (ppsNew);
   ppsNew->sock = sock;
   ppsNew->iFlags = 0;
   ppsNew->htaskOwner = GetCurrentTask();
   ppsNew->ppsNext    = ppsList;
   ppsNew->iEvents    = 0;
   ppsNew->iSocketState = 0;
   ppsNew->nOutstanding = 0;
   ppsList = ppsNew;
   return ppsNew;
  } 

static struct per_socket_info *
GetPerSockInfo( SOCKET sock )
{
    struct per_socket_info *pps;

    for (pps = ppsList; pps; pps = pps->ppsNext)
	if (pps->sock == sock )
	    break;
/* if pps is 0, return Socket error here */
    return (pps);
}         

static struct per_task_info *
GetTaskInfo ( HTASK htask)
{
    struct per_task_info *ppt;
   
    for ( ppt = pptList; ppt; ppt = ppt->pptNext)
	if (ppt->htask == htask)
	    break;
    return (ppt);
}

static BOOL
BlockingHook ( struct per_task_info *ppt )
{
  MSG msg;
  BOOL ret;
  if (ppt->lpBlockFunc ) 
     return((BOOL far pascal (*)()) ppt->lpBlockFunc)(); 
  ret = PeekMessage(&msg,(HWND)NULL,0,0,PM_REMOVE);
  if (ret)
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_QUIT )
           ppt->bCancel = TRUE;
    }
  return(ret);
}

static struct per_socket_info * CreateNewSocket ( struct per_task_info *ppt, SOCKET NewSock )
{
  struct per_socket_info  *ppsNew;
   ppsNew = (struct per_socket_info *) 
		WinMalloc (sizeof(struct per_socket_info));
   memset(ppsNew,0,sizeof(struct per_socket_info));
   ppsNew->sock  = NewSock;
   ppsNew->iFlags = 0;
   ppsNew->htaskOwner = ppt->htask;
   ppsNew->ppsNext = ppsList;
   ppsNew->iEvents = 0;
   ppsList = ppsNew;
   return ppsNew;
}


void DeleteSocket( struct per_socket_info *pps ) 

 {
   struct per_socket_info **ppps;

     for ( ppps = &ppsList; *ppps ; ppps = & ( (*ppps)->ppsNext ))
        {
          if ( *ppps == pps )
	    {
    		*ppps = pps->ppsNext;
                WinFree(pps);
                break;
            }
        }
 } 





