/*
	@(#)TwinSock.h	1.4
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
 
/* Here is what we do, We maintain a linked list on a per-task basis
   each time the WSAStartup is called, Please note here that an application
   can call WSAStartup once to invoke the winsock.dll after which only can it 
   call any socket related functions or the WSA calls. We maintain a list of
   parameters which will help us keep track of the communications and the
   socket calls made by the application
*/
#include <netdb.h> 

#define TWIN_ACCEPT WM_USER+1
#define TWIN_ASYNCSELECT WM_USER+2 
#define TWIN_GETHOSTNAME WM_USER+3 
#define TWIN_ASYNCGETSERVBYNAME WM_USER+4
#define TWIN_ASYNCGETSERVBYPORT WM_USER+5
#define TWIN_ASYNCGETHOSTBYNAME WM_USER+6
#define TWIN_ASYNCGETHOSTBYADDR WM_USER+7
#define TWIN_ASYNCGETPROTOBYNAME WM_USER+8
#define TWIN_ASYNCGETPROTOBYNUMBER WM_USER+9
#define TWIN_ASYNCCANCEL WM_USER+10

/* The Following defines are used internally to keep track as to the 
   status of a particular socket, This is used when Async Messages are to 
   be sent in response to a WSAAsyncSelect call . In a nutshell if the
   Socket is marked for Listen only then that Socket cannot send anything 
   other than an FD_ACCEPT 
*/

#define TWIN_SOCKET_STATE_ACCEPT  0x0001 
#define TWIN_SOCKET_STATE_CONNECT 0x0002
#define TWIN_SOCKET_STATE_SHUTDOWN 0x0004
#define TWIN_SOCKET_STATE_NONBLOCK 0x0008
#define TWIN_SOCKET_STATE_CLOSED   0x0010
#define TWIN_SOCKET_STATE_MUSTCONNECT 0x0020
#define TWIN_SOCKET_STATE_CONNECTING   0x0040
/* This is used to check the status of the socket */
#define TWIN_SOCKET_MASTER             0x0001
#define TWIN_SOCKET_SLAVE	       0x0002
/* This is used to determine the status of the Socket */
#define TWIN_SOCKET_STATE_ACCEPTED  0x0001 
#define TWIN_SOCKET_STATE_CONNECTED 0x0002
#define TWIN_SOCKET_STATE_READ      0x0004
#define TWIN_SOCKET_STATE_WRITTEN   0x0008
#define TWIN_SOCKET_STATE_DATAPENDING  0x0010
#define TWIN_CHECKDATAPENDING          0x0020

static struct per_task_info *pptList;
static struct per_socket_info *ppsList;
void DeleteSocketTask ( struct per_task_info * );
void DeleteSocket( struct per_socket_info *);
BOOL InitTwinWinsock ( HTASK );
LRESULT CALLBACK SockProc ( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK TwinGetHostName ( HWND, UINT, WPARAM, LPARAM );
static HANDLE ReturnWSAHandle(void);
extern HANDLE hInst;
HWND   hChildSock;

struct	per_task_info
{
	HTASK			htask;
        HWND			AsyncWndHandle;
        unsigned int            AsyncWndMsg;
	char			achAddress[16];
	struct	per_task_info	*pptNext;
	int			iErrno;
        int  			iAsyncLen;  
	FARPROC			lpBlockFunc;
	BOOL			bCancel;
	BOOL			bBlocking;
	struct servent		se;
	struct hostent		he;
	struct protoent		pe;
        char                    *name;
        char                    *proto;
	char			achHostEnt[MAXGETHOSTSTRUCT];
	char			*apchHostAlii[MAXGETHOSTSTRUCT];
	char			*apchHostAddresses[MAXGETHOSTSTRUCT];
	char			achServEnt[MAXGETHOSTSTRUCT];
	char			*apchServAlii[MAXGETHOSTSTRUCT];
        char                    *pAsyncBuffer;
	char			achProtoEnt[MAXGETHOSTSTRUCT];
	char			*apchProtoAlii[MAXGETHOSTSTRUCT]; 
};

struct	per_socket_info
{
	SOCKET			sock;
        SOCKET                  ParentSock;
	int			iFlags;
/*	struct	data		*pdIn;
	struct	data		*pdOut; */
        struct  sockaddr        *SaveSockAddr;
        int                     iLen;
        int 			iNused;
        int 			SaveSockLen;
        int                     iSocketState;
        BOOL			iWSACalled;
	HTASK			htaskOwner;
	struct	per_socket_info	*ppsNext;
	long			iEvents;
	HWND			hParentWnd;
        HWND                    hChild;
	UINT			wMsg_from_parent;
        UINT			wMsg_to_child;
        long			ret_code_from_parent;
        long 			ret_code_from_client;
	long			nOutstanding;
};

/* This struct is used by the GetDB Routines */
struct get_db_info
{
       const char *db_char_arg1;
       const char *db_char_arg2;
       int 	db_arg2;
       int 	db_arg3;
};

struct SelectInfo 
   {
       fd_set *read_fds_info;
       fd_set *write_fds_info;
       fd_set *except_fds_info;
       struct timeval *Select_Time_Value;
       unsigned int  sock;
   };


/* This is used to maintain information about a socket on a per-socket
   basis, Open many sockets per task ??  
*/
struct sock_data
  {
     SOCKET sock;
     char   *SockData;
     int    DataLen;
     int    DataFlags;
     struct sockaddr *from_addr;
     int    *from_len;
     int    to_len;
  };

