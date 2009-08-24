
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define NOAPIPROTO
#include "windows.h"
#include "Willows.h"

#include "WinDefs.h"
#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "Resources.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "ModTable.h"


extern long int Accept(); 
extern long int Bind(); 
extern long int Closesocket(); 
extern long int Connect(); 
extern long int Getpeername(); 
extern long int Getsockname(); 
extern long int Getsockopt(); 
extern long int Htonl(); 
extern long int Htons(); 
extern long int Inet_addr(); 
extern long int Inet_ntoa(); 
extern long int Ioctlsocket(); 
extern long int Listen(); 
extern long int Ntohl(); 
extern long int Ntohs(); 
extern long int Recv(); 
extern long int Recvfrom(); 
extern long int Select(); 
extern long int Send(); 
extern long int Sendto(); 
extern long int Setsockopt(); 
extern long int Shutdown(); 
extern long int Socket(); 
extern long int Gethostbyaddr(); 
extern long int Gethostbyname(); 
extern long int Getprotobyname(); 
extern long int Getprotobynumber(); 
extern long int Getservbyname(); 
extern long int Getservbyport(); 
extern long int Gethostname(); 
extern long int WSAAsyncSelect(); 
extern long int WSAAsyncGetHostByAddr(); 
extern long int WSAAsyncGetHostByName(); 
extern long int WSAAsyncGetProtoByNumber(); 
extern long int WSAAsyncGetProtoByName(); 
extern long int WSAAsyncGetServByPort(); 
extern long int WSAAsyncGetServByName(); 
extern long int WSACancelAsyncRequest(); 
extern long int WSASetBlockingHook(); 
extern long int WSAUnhookBlockingHook(); 
extern long int WSAGetLastError(); 
extern long int WSASetLastError(); 
extern long int WSACancelBlockingCall(); 
extern long int WSAIsBlocking(); 
extern long int WSAStartup(); 
extern long int WSACleanup(); 
extern long int __WSAFDIsSet(); 
extern long int WEP(); 

 extern long int Trap();
 extern long int IT_WSASTARTUP();
 extern long int IT_ACCEPT();
 extern long int IT_BIND();
 extern long int IT_CLOSESOCKET();
 extern long int IT_CONNECT();
 extern long int IT_GETPEERNAME();
 extern long int IT_GETSOCKNAME();
 extern long int IT_GETSOCKOPT();
 extern long int IT_HTONS();
 extern long int IT_INET_ADDR();
 extern long int IT_INET_NTOA();
 extern long int IT_IOCTLSOCKET();
 extern long int IT_SOCKET();
 extern long int IT_LISTEN();
 extern long int IT_NTOHS();
 extern long int IT_NTOHL();
 extern long int IT_HTONL();
 extern long int IT_HTONS();
 extern long int IT_RECV();
 extern long int IT_RECVFROM();
 extern long int IT_SELECT();
 extern long int IT_SEND();
 extern long int IT_SENDTO();
 extern long int IT_SETSOCKOPT();
 extern long int IT_SHUTDOWN();
 extern long int IT_GETHOSTBYADDR();
 extern long int IT_GETHOSTBYNAME();
 extern long int IT_GETPROTOBYNAME();
 extern long int IT_GETPROTOBYNUMBER();
 extern long int IT_GETSERVBYNAME();
 extern long int IT_GETSERVBYPORT();
 extern long int IT_WSAASYNCGETHOSTBYADDR();
 extern long int IT_WSAASYNCGETHOSTBYNAME();
 extern long int IT_WSAASYNCGETPROTOBYNAME();
 extern long int IT_WSAASYNCGETPROTOBYNUMBER();
 extern long int IT_WSAASYNCGETSERVBYNAME();
 extern long int IT_WSAASYNCGETSERVBYPORT();
 extern long int IT_WSAASYNCSELECT();
 extern long int IT_WSACANCELASYNCREQUEST();
 extern long int IT_WSACLEANUP();
 extern long int IT_WSAGETLASTERROR();
 extern long int IT_WSAISBLOCKING();
 extern long int IT_WSASETBLOCKINGHOOK();
 extern long int IT_WSASETLASTERROR();
 extern long int IT_WSACANCELBLOCKINGCALL();
 extern long int IT_WSAUNHOOKBLOCKINGHOOK();
 extern long int IT_GETHOSTNAME();
 extern long int IT__WSAFDISSET();


 static long int (*seg_image_WINSOCK_O[])() =
 {	/* nil */	0, 0,
 	/* 001 */	IT_ACCEPT, Accept,
 	/* 002 */	IT_BIND, Bind,
 	/* 003 */	IT_CLOSESOCKET, Closesocket,
 	/* 004 */	IT_CONNECT, Connect,
 	/* 005 */	IT_GETPEERNAME, Getpeername,
 	/* 006 */	IT_GETSOCKNAME, Getsockname,
 	/* 007 */	IT_GETSOCKOPT, Getsockopt,
 	/* 008 */	IT_HTONL,Htonl,
 	/* 009 */	IT_HTONS, Htons,
 	/* 00a */	IT_INET_ADDR, Inet_addr,
 	/* 00b */	IT_INET_NTOA, Inet_ntoa,
 	/* 00c */	IT_IOCTLSOCKET, Ioctlsocket,
 	/* 00d */	IT_LISTEN, Listen,
 	/* 00e */	IT_NTOHL, Ntohl,
 	/* 00f */	IT_NTOHS, Ntohs,
 	/* 010 */	IT_RECV, Recv,
 	/* 011 */	IT_RECVFROM, Recvfrom,
 	/* 012 */	IT_SELECT, Select,
 	/* 013 */	IT_SEND, Send,
 	/* 014 */	IT_SENDTO, Sendto,
 	/* 015 */	IT_SETSOCKOPT, Setsockopt,
 	/* 016 */	IT_SHUTDOWN, Shutdown,
 	/* 017 */	IT_SOCKET, Socket,
	/* 018 */	Trap, 0,
	/* 019 */	Trap, 0,
	/* 01a */	Trap, 0,
	/* 01b */	Trap, 0,
	/* 01c */	Trap, 0,
	/* 01d */	Trap, 0,
	/* 01e */	Trap, 0,
	/* 01f */	Trap, 0,
	/* 020 */	Trap, 0,
	/* 021 */	Trap, 0,
	/* 022 */	Trap, 0,
	/* 023 */	Trap, 0,
	/* 024 */	Trap, 0,
	/* 025 */	Trap, 0,
	/* 026 */	Trap, 0,
	/* 027 */	Trap, 0,
	/* 028 */	Trap, 0,
	/* 029 */	Trap, 0,
	/* 02a */	Trap, 0,
	/* 02b */	Trap, 0,
	/* 02c */	Trap, 0,
	/* 02d */	Trap, 0,
	/* 02e */	Trap, 0,
	/* 02f */	Trap, 0,
	/* 030 */	Trap, 0,
	/* 031 */	Trap, 0,
	/* 032*/	Trap, 0,
 	/* 033 */	IT_GETHOSTBYADDR, Gethostbyaddr,
 	/* 034 */	IT_GETHOSTBYNAME, Gethostbyname,
 	/* 035 */	IT_GETPROTOBYNAME, Getprotobyname,
 	/* 036 */	IT_GETPROTOBYNUMBER, Getprotobynumber,
 	/* 037 */	IT_GETSERVBYNAME, Getservbyname,
 	/* 038 */	IT_GETSERVBYPORT,Getservbyport, 
 	/* 039 */	IT_GETHOSTNAME,Gethostname, 
	/* 03a */	Trap, 0,
	/* 03b */	Trap, 0,
	/* 03c */	Trap, 0,
	/* 03d */	Trap, 0,
	/* 03e */	Trap, 0,
	/* 03f */	Trap, 0,
	/* 040 */	Trap, 0,
	/* 041 */	Trap, 0,
	/* 042 */	Trap, 0,
	/* 043 */	Trap, 0,
	/* 044 */	Trap, 0,
	/* 045 */	Trap, 0,
	/* 046 */	Trap, 0,
	/* 047 */	Trap, 0,
	/* 048 */	Trap, 0,
	/* 049 */	Trap, 0,
	/* 04a */	Trap, 0,
	/* 04b */	Trap, 0,
	/* 04c */	Trap, 0,
	/* 04d */	Trap, 0,
	/* 04e */	Trap, 0,
	/* 04f */	Trap, 0,
	/* 050 */	Trap, 0,
	/* 051 */	Trap, 0,
	/* 052 */	Trap, 0,
	/* 053 */	Trap, 0,
	/* 054 */	Trap, 0,
	/* 055 */	Trap, 0,
	/* 056 */	Trap, 0,
	/* 057 */	Trap, 0,
	/* 058 */	Trap, 0,
	/* 059 */	Trap, 0,
	/* 05a */	Trap, 0,
	/* 05b */	Trap, 0,
	/* 05c */	Trap, 0,
	/* 05d */	Trap, 0,
	/* 05e */	Trap, 0,
	/* 05f */	Trap, 0,
	/* 060 */	Trap, 0,
	/* 061 */	Trap, 0,
	/* 062 */	Trap, 0,
	/* 063 */	Trap, 0,
	/* 064 */	Trap, 0,
 	/* 065 */	IT_WSAASYNCSELECT, WSAAsyncSelect,
 	/* 066 */	IT_WSAASYNCGETHOSTBYADDR, WSAAsyncGetHostByAddr,
 	/* 067 */	IT_WSAASYNCGETHOSTBYNAME, WSAAsyncGetHostByName,
 	/* 068 */	IT_WSAASYNCGETPROTOBYNUMBER, WSAAsyncGetProtoByNumber,
 	/* 069 */	IT_WSAASYNCGETPROTOBYNAME, WSAAsyncGetProtoByName,
 	/* 06a */	IT_WSAASYNCGETSERVBYPORT, WSAAsyncGetServByPort, 
 	/* 06b */	IT_WSAASYNCGETSERVBYNAME, WSAAsyncGetServByName,
 	/* 06c */	IT_WSACANCELASYNCREQUEST, WSACancelAsyncRequest,
 	/* 06d */	IT_WSASETBLOCKINGHOOK, WSASetBlockingHook,
 	/* 06e */	IT_WSAUNHOOKBLOCKINGHOOK, WSAUnhookBlockingHook,
 	/* 06f */	IT_WSAGETLASTERROR, WSAGetLastError,
 	/* 070 */	IT_WSASETLASTERROR, WSASetLastError,
 	/* 071 */	IT_WSACANCELBLOCKINGCALL, WSACancelBlockingCall,
 	/* 072 */	IT_WSAISBLOCKING, WSAIsBlocking,
 	/* 073 */	IT_WSASTARTUP, WSAStartup,
 	/* 074 */	IT_WSACLEANUP, WSACleanup,
	/* 075 */	Trap, 0,
	/* 076 */	Trap, 0,
	/* 077 */	Trap, 0,
	/* 078 */	Trap, 0,
	/* 079 */	Trap, 0,
	/* 07a */	Trap, 0,
	/* 07b */	Trap, 0,
	/* 07c */	Trap, 0,
	/* 07d */	Trap, 0,
	/* 07e */	Trap, 0,
	/* 07f */	Trap, 0,
	/* 080 */	Trap, 0,
	/* 081 */	Trap, 0,
	/* 082 */	Trap, 0,
	/* 083 */	Trap, 0,
	/* 084 */	Trap, 0,
	/* 085 */	Trap, 0,
	/* 086 */	Trap, 0,
	/* 087 */	Trap, 0,
	/* 088 */	Trap, 0,
	/* 089 */	Trap, 0,
	/* 08a */	Trap, 0,
	/* 08b */	Trap, 0,
	/* 08c */	Trap, 0,
	/* 08d */	Trap, 0,
	/* 08e */	Trap, 0,
	/* 08f */	Trap, 0,
	/* 090 */	Trap, 0,
	/* 091 */	Trap, 0,
	/* 092 */	Trap, 0,
	/* 093 */	Trap, 0,
	/* 094 */	Trap, 0,
	/* 095 */	Trap, 0,
	/* 096 */	Trap, 0,
	/* 097 */	IT__WSAFDISSET, __WSAFDIsSet,
 	0
 };

 
  /* Segment Table WINSOCK */
 
 SEGTAB SegmentTableWINSOCK[] =
 {	{ (char *) seg_image_WINSOCK_O, 1208, TRANSFER_CALLBACK, 1208, 0, 0 },
 	/* end */	{ 0, 0, 0, 0, 0, 0 }
 };
