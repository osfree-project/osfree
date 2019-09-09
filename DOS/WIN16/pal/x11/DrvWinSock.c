/*
	@(#)DrvWinSock.c	1.22
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
#include "windowsx.h"
#include "DrvWinSock.h"
#include "Driver.h"
#include "Log.h"
#include "WinSockUtil.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "errno.h"

DWORD DrvWinsockInit(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockAccept(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockBind(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockCloseSocket(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockConnect(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetPeerName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetSockName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetSockOpt(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockHTONL(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockHTONS(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockInetAddr(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockInetNTOA(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockIOCTLSocket(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockListen(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockNTOHL(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockNTOHS(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockRecv(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockRecvFrom(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockSelect(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockSend(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockSendTo(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockSetSockOpt(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockShutdown(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockSocket(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetHostByAddr(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetHostByName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetProtoByName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetProtoByNumber(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetServByName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetServByPort(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockGetHostName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAAsyncSelect(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAAsyncGetHostByAddr(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAAsyncGetHostByName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAAsyncGetProtoByNumber(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAAsyncGetProtoByName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAAsyncGetServByPort(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAAsyncGetServByName(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSACancelAsyncRequest(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSASetBlockingHook(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAUnhookBlockingHook(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAGetLastError(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSASetLastError(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSACancelBlockingHook(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAIsBlocking(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAStartup(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSACleanup(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockWSAFDISSet(LPARAM,LPARAM,LPVOID);
static DWORD DrvWinsockDoNothing(LPARAM,LPARAM,LPVOID);
DWORD	DrvWinsockTab(void);

unsigned int TWIN_WSAError;

static TWINDRVSUBPROC DrvWinSockEntryTab[] = {
	DrvWinsockInit,
	DrvWinsockDoNothing,
	DrvWinsockDoNothing,
	DrvWinsockAccept,
	DrvWinsockBind,
	DrvWinsockCloseSocket,
	DrvWinsockConnect,
	DrvWinsockGetPeerName,
	DrvWinsockGetSockName,
	DrvWinsockGetSockOpt,
	DrvWinsockHTONL,
	DrvWinsockHTONS,
	DrvWinsockInetAddr,
	DrvWinsockInetNTOA,
	DrvWinsockIOCTLSocket,
	DrvWinsockListen,
	DrvWinsockNTOHL,
	DrvWinsockNTOHS,
	DrvWinsockRecv,
	DrvWinsockRecvFrom,
	DrvWinsockSelect,
	DrvWinsockSend,
	DrvWinsockSendTo,
	DrvWinsockSetSockOpt,
	DrvWinsockShutdown,
	DrvWinsockSocket,
	DrvWinsockGetHostByAddr,
	DrvWinsockGetHostByName,
	DrvWinsockGetProtoByName,
	DrvWinsockGetProtoByNumber,
	DrvWinsockGetServByName,
	DrvWinsockGetServByPort,
	DrvWinsockGetHostName,
	DrvWinsockWSAAsyncSelect,
	DrvWinsockWSAAsyncGetHostByAddr,
	DrvWinsockWSAAsyncGetHostByName,
	DrvWinsockWSAAsyncGetProtoByNumber,
	DrvWinsockWSAAsyncGetProtoByName,
	DrvWinsockWSAAsyncGetServByPort,
	DrvWinsockWSAAsyncGetServByName,
	DrvWinsockWSACancelAsyncRequest,
	DrvWinsockWSASetBlockingHook,
	DrvWinsockWSAUnhookBlockingHook,
	DrvWinsockWSAGetLastError,
	DrvWinsockWSASetLastError,
	DrvWinsockWSACancelBlockingHook,
	DrvWinsockWSAIsBlocking,
	DrvWinsockWSAStartup,
	DrvWinsockWSACleanup,
	DrvWinsockWSAFDISSet
};

DWORD
DrvWinsockInit(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)TRUE;
}

static DWORD
DrvWinsockDoNothing(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return 0L;
}

static DWORD
DrvWinsockAccept(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	SOCKET Sock;
        int rc;
	LPSOCKADDR_IN TWSock_addr;
	
	Sock = 0;
        TWSock_addr = (LPSOCKADDR_IN) lpStruct;
        rc = fcntl(dwParm1,F_SETFL,O_NONBLOCK);
        if (rc < 0 ) return(INVALID_SOCKET);
        if((rc =  accept(dwParm1,(struct sockaddr *)TWSock_addr,(int*)dwParm2) < 0 ))
        {
		TWIN_GetWSAError();
		return (DWORD)rc;
	}
	else {
		return (DWORD)rc;
	}
}

static DWORD
DrvWinsockBind(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	LPSOCKADDR_IN TWSock_addr;
	int rc;

	TWSock_addr = (LPSOCKADDR_IN ) lpStruct;
	rc = (int) bind(dwParm1,(const struct sockaddr *)TWSock_addr,dwParm2);
	if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
	return (DWORD)rc;
}

static DWORD
DrvWinsockCloseSocket(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)close(dwParm1);
}

static DWORD
DrvWinsockConnect(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	LPSOCKADDR_IN TWSock_addr;
	int rc;
	
	TWSock_addr = (LPSOCKADDR_IN ) lpStruct;
		rc =  (int)connect(dwParm1,(const struct sockaddr *)TWSock_addr,dwParm2);
	if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
	return (DWORD)rc;
}

static DWORD
DrvWinsockGetPeerName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	LPSOCKADDR_IN TWSock_addr;
	int rc;
	
	TWSock_addr = (LPSOCKADDR_IN ) lpStruct;
        rc =  (int)getpeername(dwParm1,(struct sockaddr *)TWSock_addr,
		(int *)dwParm2);
        if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
        return (DWORD)rc;
}

static DWORD
DrvWinsockGetSockName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	LPSOCKADDR_IN TWSock_addr;
	int rc;

	TWSock_addr = (LPSOCKADDR_IN) lpStruct;
	rc = (int)getsockname(dwParm1,(struct sockaddr *)TWSock_addr,
		(int*)dwParm2);
	if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
	return (DWORD)rc;
}

static DWORD
DrvWinsockGetSockOpt(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	struct get_db_info *pdb_info;	
	int rc;
	
        pdb_info = (struct get_db_info*)lpStruct;
        switch ( pdb_info->db_arg3 )
                {
                   case 1:
                      pdb_info->db_arg3 = SOL_SOCKET;
                   switch ( pdb_info->db_arg2 )
                         {
                           case 0x01: pdb_info->db_arg2 = SO_DEBUG;
                                      break;
                           case 0x04: pdb_info->db_arg2 = SO_REUSEADDR;
                                      break;
                           case 0x08: pdb_info->db_arg2 = SO_KEEPALIVE;
                                      break;
                           case 0x10: pdb_info->db_arg2 = SO_DONTROUTE;
                                      break;
                           case 0x20: pdb_info->db_arg2 = SO_BROADCAST;
                                      break;
                           case 0x80: pdb_info->db_arg2 = SO_LINGER;
                                      break;
                           case 0x100: pdb_info->db_arg2 = SO_OOBINLINE;
                                      break;
                           case 0x1001: pdb_info->db_arg2 = SO_SNDBUF;
				      break;
                           case 0x1002: pdb_info->db_arg2 = SO_RCVBUF;
                                      break;
                           case 0x1007: pdb_info->db_arg2 = SO_ERROR;
                                      break;
                           case 0x1008: pdb_info->db_arg2 = SO_TYPE;
                                      break;
                           default:
                                      break;
                           }
                           break;

                    case 6: pdb_info->db_arg2 = IPPROTO_TCP;
                   }
            rc = (int)getsockopt((SOCKET)dwParm1,(int)pdb_info->db_arg3, 
		(int)pdb_info->db_arg2,(char *)pdb_info->db_char_arg1,
		(int*)dwParm2);
            if ( rc < 0 ) TWIN_GetWSAError();
            return (DWORD)rc;
}

static DWORD
DrvWinsockHTONL(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)htonl(dwParm1);
}

static DWORD
DrvWinsockHTONS(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)htons(dwParm1);
}

static DWORD
DrvWinsockInetAddr(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)inet_addr((char*)dwParm1);
}

static DWORD
DrvWinsockInetNTOA(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	char * ctlname;
	struct in_addr Drv_in, *PDrv_in;

	ctlname = 0;
	PDrv_in = (struct in_addr*)lpStruct;
	Drv_in.s_addr = (unsigned long)PDrv_in->s_addr;
	ctlname=(char *)inet_ntoa(Drv_in);
	return (DWORD)ctlname;
}

static DWORD
DrvWinsockIOCTLSocket(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	long newcmd;
	char * ctlname;
	unsigned long *newargp;

          newcmd = dwParm2;
          newargp = (unsigned long *)lpStruct;
          ctlname =0;
          if ( dwParm2  == _IOR('f',127,unsigned long) )
            {
                ctlname = "FIONREAD";
                newcmd=FIONREAD;
             }
          else
          if (dwParm2 == _IOW('f',126,unsigned long)  || 
		dwParm2 == _IOR('f',126,unsigned long))
             {
                ctlname = "FIONBIO";
                newcmd  = FIONBIO;
              }
           else
           if (dwParm2 == _IOW('f',125,unsigned long) )
              {
                ctlname = "FIOASYNC";
                newcmd =  FIOASYNC;
               }
            if ( ioctl(dwParm1,newcmd,newargp) < 0 ) {
                TWIN_GetWSAError();
                return -1;
            }
            lpStruct = newargp;
            return (DWORD)0;
}

static DWORD
DrvWinsockListen(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	int rc;
	
	rc = (int)listen((SOCKET)dwParm1,(int)dwParm2);
	if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
	return (DWORD)rc;
}

static DWORD
DrvWinsockNTOHL(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)ntohl(dwParm1);
}

static DWORD
DrvWinsockNTOHS(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)ntohs((WORD)dwParm1);
}

static DWORD
DrvWinsockRecv(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	/* We should never execute this function */
	return (DWORD)0;
}

static DWORD
DrvWinsockRecvFrom(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	struct sock_data *ReadSendData;
	int rc;
	
            ReadSendData = (struct sock_data *)dwParm2;
            rc = fcntl(dwParm1,F_SETFL,O_NONBLOCK);
            if (rc < 0 ) return(INVALID_SOCKET);
            if ( ReadSendData->from_addr && ReadSendData->from_len)
            	rc = (int) recvfrom((SOCKET) ReadSendData->sock, 
			(char *) ReadSendData->SockData, 
			(int)ReadSendData->DataLen, 
			(int)ReadSendData->DataFlags,
			(struct sockaddr *)ReadSendData->from_addr,
			(int *)ReadSendData->from_len);
            else
       		rc = (int) recv((SOCKET) ReadSendData->sock, 
			(char *) ReadSendData->SockData, 
			(int)ReadSendData->DataLen, 
			(int) ReadSendData->DataFlags);
            if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
            return (DWORD)rc;
}

static DWORD
DrvWinsockSelect(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	struct SelectInfo *Select_info_drv;
	SOCKET Sock;
	long SelectRC;

	Select_info_drv = (struct SelectInfo *) lpStruct;
	Sock = 0xffff;
	SelectRC = select(Sock,Select_info_drv->read_fds_info,
			Select_info_drv->write_fds_info,
			Select_info_drv->except_fds_info,
			Select_info_drv->Select_time_out);
	return (DWORD)SelectRC;
}

static DWORD
DrvWinsockSend(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	/* We should never execute this function */
	return (DWORD)0;
}

static DWORD
DrvWinsockSendTo(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	struct sock_data   *ReadSendData;
	int rc;

	ReadSendData = (struct sock_data *)dwParm2;
	if ( ReadSendData->from_addr && ReadSendData->to_len)
		rc = (int) sendto((SOCKET) ReadSendData->sock, 
			(char *) ReadSendData->SockData,
			(int)ReadSendData->DataLen, 
			(int)ReadSendData->DataFlags,
			(struct sockaddr *)ReadSendData->from_addr,
			(int)ReadSendData->to_len);
        else
		rc = (int) send((SOCKET) ReadSendData->sock, 
			(char *) ReadSendData->SockData, 
			(int)ReadSendData->DataLen, 
			(int)ReadSendData->DataFlags);
	if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
	return (DWORD)rc;
}

static DWORD
DrvWinsockSetSockOpt(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
            struct get_db_info *pdb_info;
            int rc;	
            pdb_info = (struct get_db_info*)lpStruct;
            switch ( dwParm2 )
                 {
                   case 1:
                      dwParm2 = SOL_SOCKET;
                   switch ( pdb_info->db_arg2 )
                         {
                           case 0x01: pdb_info->db_arg2 = SO_DEBUG;
                                      break;
                           case 0x04: pdb_info->db_arg2 = SO_REUSEADDR;
                                      break;
                           case 0x08: pdb_info->db_arg2 = SO_KEEPALIVE;
                                      break;
                           case 0x10: pdb_info->db_arg2 = SO_DONTROUTE;
                                      break;
                           case 0x20: pdb_info->db_arg2 = SO_BROADCAST;
                                      break;
                           case 0x80: pdb_info->db_arg2 = SO_LINGER;
                                      break;
                           case 0x100: pdb_info->db_arg2 = SO_OOBINLINE;
                                      break;
                           case 0x1001: pdb_info->db_arg2 = SO_SNDBUF;
                                      break;
                           case 0x1002: pdb_info->db_arg2 = SO_RCVBUF;
                                      break;
                           case 0x1007: pdb_info->db_arg2 = SO_ERROR;
                                      break;
                           case 0x1008: pdb_info->db_arg2 = SO_TYPE;
                                      break;
                           case 0xff7f: pdb_info->db_arg2 = ~SO_LINGER;
                                      break;
                           default:
                                      break;
                           }
                           break;
                    case 6: pdb_info->db_arg2 = IPPROTO_TCP;
                   }
            rc = (int) setsockopt((SOCKET)dwParm1,(int) dwParm2, 
			(int)pdb_info->db_arg2,
			(const char *)pdb_info->db_char_arg1, 
			(int)pdb_info->db_arg3);
            if ( rc < 0 ) TWIN_GetWSAError();
            return (DWORD)rc;
}

static DWORD
DrvWinsockShutdown(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)shutdown((unsigned long)dwParm1,(int)dwParm2);
}

static DWORD
DrvWinsockSocket(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)socket((int)dwParm1,(int)dwParm2,(int)lpStruct);
}


static DWORD
DrvWinsockGetHostByAddr(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	  struct get_db_info *pdb_info = (struct get_db_info *)dwParm2;
        PHOSTENT he;

        he = gethostbyaddr((const char *)pdb_info->db_char_arg1,
		(int)pdb_info->db_arg2,(int)pdb_info->db_arg3);
        return (DWORD)he;
}

static DWORD
DrvWinsockGetHostByName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        struct get_db_info *pdb_info = (struct get_db_info *)dwParm2;
        PHOSTENT he;
        he = gethostbyname((const char *)pdb_info->db_char_arg1);
        return (DWORD)he;    
}

static DWORD
DrvWinsockGetProtoByName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        struct get_db_info *pdb_info = (struct get_db_info *)dwParm2;
	  PPROTOENT pe;

	  pe = getprotobyname((const char *)pdb_info->db_char_arg1);
        return (DWORD)pe;
}

static DWORD
DrvWinsockGetProtoByNumber(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	  struct get_db_info *pdb_info = (struct get_db_info *)dwParm2;
        PPROTOENT pe;

        pe = getprotobynumber((int)pdb_info->db_arg2);
        return (DWORD)pe;
}

static DWORD
DrvWinsockGetServByName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        struct get_db_info *pdb_info = (struct get_db_info *)dwParm2;
	  PSERVENT se;
	
	  se = getservbyname((const char *)pdb_info->db_char_arg1,
				(const char *)pdb_info->db_char_arg2);
        return (DWORD)se;
}

static DWORD
DrvWinsockGetServByPort(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	  struct get_db_info *pdb_info = (struct get_db_info *)dwParm2;
        PSERVENT se;

        se = getservbyport((int)pdb_info->db_arg2,
		(const char *)pdb_info->db_char_arg2);
        return (DWORD)se;
}

static DWORD
DrvWinsockGetHostName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	  int rc;
	
        rc = (int)gethostname((char *)dwParm1, (int) dwParm2 );
        if ( rc == SOCKET_ERROR) TWIN_GetWSAError();
        return (DWORD)rc;
}


static DWORD
DrvWinsockWSAAsyncSelect(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	    fd_set read_fds, write_fds, except_fds;
	    SOCKET Sock;
            long event, SelectRC;
            struct timeval Select_Time_Out;
	    
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&except_fds);
            Sock = (SOCKET) dwParm1;
            event = (long) dwParm2;
             if ( event & FD_READ )
                FD_SET(Sock,&read_fds);
             if ( event & FD_WRITE )
                FD_SET(Sock,&write_fds);
            if (event &FD_ACCEPT)
                {
                FD_SET(Sock,&read_fds);
                FD_SET(Sock,&write_fds);
                }
            if (event & TWIN_CHECKDATAPENDING )
                {
                  FD_SET(Sock,&read_fds);
                  FD_SET(Sock,&write_fds);
                  FD_SET(Sock,&except_fds);
                }
            fcntl(Sock,F_SETFL,O_NONBLOCK);
                  Select_Time_Out.tv_sec = 0;
                  Select_Time_Out.tv_usec = 0;
             SelectRC = select(Sock+1,&read_fds,&write_fds,&except_fds,
			(event &TWIN_CHECKDATAPENDING) ? &Select_Time_Out : &Select_Time_Out);
            event = 0;
            if (FD_ISSET(Sock,&read_fds))
                   event |= FD_READ;
            if (FD_ISSET(Sock,&write_fds))
                   event |= FD_WRITE;
            return (DWORD)event;
}

static DWORD
DrvWinsockWSAAsyncGetHostByAddr(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAAsyncGetHostByName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

#ifdef 0
static DWORD
DrvWinsockWSAAsyncGetHostByNumber(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}
#endif

static DWORD
DrvWinsockWSAAsyncGetProtoByNumber(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAAsyncGetProtoByName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAAsyncGetServByPort(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAAsyncGetServByName(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSACancelAsyncRequest(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSASetBlockingHook(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAUnhookBlockingHook(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAGetLastError(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)TWIN_WSAError;
}

static DWORD
DrvWinsockWSASetLastError(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	TWIN_WSAError = (int)dwParm1;
	return (DWORD)0;
}

static DWORD
DrvWinsockWSACancelBlockingHook(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}
	
static DWORD
DrvWinsockWSAIsBlocking(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAStartup(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSACleanup(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

static DWORD
DrvWinsockWSAFDISSet(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	return (DWORD)0;
}

DWORD
DrvWinsockTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvWinSockEntryTab[0] = DrvWinsockInit;
        DrvWinSockEntryTab[1] = DrvWinsockDoNothing;
        DrvWinSockEntryTab[2] = DrvWinsockDoNothing;
        DrvWinSockEntryTab[3] = DrvWinsockAccept;
        DrvWinSockEntryTab[4] = DrvWinsockBind;
        DrvWinSockEntryTab[5] = DrvWinsockCloseSocket;
        DrvWinSockEntryTab[6] = DrvWinsockConnect;
        DrvWinSockEntryTab[7] = DrvWinsockGetPeerName;
        DrvWinSockEntryTab[8] = DrvWinsockGetSockName;
        DrvWinSockEntryTab[9] = DrvWinsockGetSockOpt;
        DrvWinSockEntryTab[10] = DrvWinsockHTONL;
        DrvWinSockEntryTab[11] = DrvWinsockHTONS;
        DrvWinSockEntryTab[12] = DrvWinsockInetAddr;
        DrvWinSockEntryTab[13] = DrvWinsockInetNTOA;
        DrvWinSockEntryTab[14] = DrvWinsockIOCTLSocket;
        DrvWinSockEntryTab[15] = DrvWinsockListen;
        DrvWinSockEntryTab[16] = DrvWinsockNTOHL;
        DrvWinSockEntryTab[17] = DrvWinsockNTOHS;
        DrvWinSockEntryTab[18] = DrvWinsockRecv;
        DrvWinSockEntryTab[19] = DrvWinsockRecvFrom;
        DrvWinSockEntryTab[20] = DrvWinsockSelect;
        DrvWinSockEntryTab[21] = DrvWinsockSend;
        DrvWinSockEntryTab[22] = DrvWinsockSendTo;
        DrvWinSockEntryTab[23] = DrvWinsockSetSockOpt;
        DrvWinSockEntryTab[24] = DrvWinsockShutdown;
        DrvWinSockEntryTab[25] = DrvWinsockSocket;
        DrvWinSockEntryTab[26] = DrvWinsockGetHostByAddr;
        DrvWinSockEntryTab[27] = DrvWinsockGetHostByName;
        DrvWinSockEntryTab[28] = DrvWinsockGetProtoByName;
        DrvWinSockEntryTab[29] = DrvWinsockGetProtoByNumber;
        DrvWinSockEntryTab[30] = DrvWinsockGetServByName;
        DrvWinSockEntryTab[31] = DrvWinsockGetServByPort;
        DrvWinSockEntryTab[32] = DrvWinsockGetHostName;
        DrvWinSockEntryTab[33] = DrvWinsockWSAAsyncSelect;
        DrvWinSockEntryTab[34] = DrvWinsockWSAAsyncGetHostByAddr;
        DrvWinSockEntryTab[35] = DrvWinsockWSAAsyncGetHostByName;
        DrvWinSockEntryTab[36] = DrvWinsockWSAAsyncGetProtoByNumber;
        DrvWinSockEntryTab[37] = DrvWinsockWSAAsyncGetProtoByName;
        DrvWinSockEntryTab[38] = DrvWinsockWSAAsyncGetServByPort;
        DrvWinSockEntryTab[39] = DrvWinsockWSAAsyncGetServByName;
        DrvWinSockEntryTab[40] = DrvWinsockWSACancelAsyncRequest;
        DrvWinSockEntryTab[41] = DrvWinsockWSASetBlockingHook;
        DrvWinSockEntryTab[42] = DrvWinsockWSAUnhookBlockingHook;
        DrvWinSockEntryTab[43] = DrvWinsockWSAGetLastError;
        DrvWinSockEntryTab[44] = DrvWinsockWSASetLastError;
        DrvWinSockEntryTab[45] = DrvWinsockWSACancelBlockingHook;
        DrvWinSockEntryTab[46] = DrvWinsockWSAIsBlocking;
        DrvWinSockEntryTab[47] = DrvWinsockWSAStartup;
        DrvWinSockEntryTab[48] = DrvWinsockWSACleanup;
        DrvWinSockEntryTab[49] = DrvWinsockWSAFDISSet;
#endif
	return (DWORD)DrvWinSockEntryTab;
}

