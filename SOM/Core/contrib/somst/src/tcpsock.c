/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

#include <rhbopt.h>
#include <rhbsomex.h>
#include <somref.h>
#include <snglicls.h>
#include <soms.h>
#include <signal.h>

#ifdef _WIN32
#	ifdef HAVE_WS2IP6_H
#		include <ws2ip6.h>
#	endif
#	ifdef HAVE_WS2TCPIP_H
#		include <ws2tcpip.h>
#	endif
#else
#	include <errno.h>
#	include <unistd.h>
#	ifndef INVALID_SOCKET
		typedef int SOCKET;
#		define INVALID_SOCKET		((SOCKET)-1)
#	endif
#endif

#define SOM_Module_tcpsock_Source
#define TCPIPSockets_Class_Source
#include <tcpsock.ih>
#include <rhbsomex.h>
#include <somserr.h>

#ifdef _WIN32
#	define TCPIPGetLastError()      Sockets_GetError(WSAGetLastError())
#else
#	define TCPIPGetLastError()      Sockets_GetError(errno)
#endif

#ifdef SIGPIPE
	#if !defined(USE_THREADS)
		#define CATCH_SIGPIPE
		static void catch_sigpipe(int i)
		{
		}
	#endif
#endif

#ifdef CATCH_SIGPIPE
#	ifdef SA_RESTART
#		define CATCH_SIGPIPE_FLAGS			SA_RESTART
#	else
#		define CATCH_SIGPIPE_FLAGS			0
#	endif
#	define CATCH_SIGPIPE_BEGIN    {						\
		static struct sigaction saBlank;				\
		struct sigaction saNew,saOld;					\
		sigset_t oldMask,newMask;						\
		saNew=saBlank;									\
		saNew.sa_flags=CATCH_SIGPIPE_FLAGS;				\
		saNew.sa_handler=catch_sigpipe;					\
		sigfillset(&newMask);							\
		sigdelset(&newMask,SIGPIPE);					\
		sigprocmask(SIG_SETMASK,&newMask,&oldMask);
#	define CATCH_SIGPIPE_END							\
		sigaction(SIGPIPE,&saOld,NULL);					\
		sigprocmask(SIG_SETMASK,&oldMask,NULL); }
#else
#	define CATCH_SIGPIPE_BEGIN
#	define CATCH_SIGPIPE_END
#endif


#define TCPIP_EXIT_RC(x)   if (x==-1) { TCPIPSockets__set_serrno(somSelf,ev,TCPIPGetLastError()); }
#define TCPIP_EXIT_FD(x)   if (x==INVALID_SOCKET) { TCPIPSockets__set_serrno(somSelf,ev,TCPIPGetLastError()); }

/* overridden methods for ::TCPIPSockets */
/* overridden method ::Sockets::somsAccept */
SOM_Scope long SOMLINK tcpsock_somsAccept(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	socklen_t nl=*namelen;
	SOCKET fd=accept(s,name,&nl);
	*namelen=nl;

	TCPIP_EXIT_FD(fd);

	return (long)fd;
}

/* overridden method ::Sockets::somsBind */
SOM_Scope long SOMLINK tcpsock_somsBind(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_sockaddr *name,
	/* in */ long namelen)
{
	long rc=bind(s,name,namelen);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsConnect */
SOM_Scope long SOMLINK tcpsock_somsConnect(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_sockaddr *name,
	/* in */ long namelen)
{
	long rc=connect(s,name,namelen);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsGethostbyaddr */
SOM_Scope Sockets_hostent *SOMLINK tcpsock_somsGethostbyaddr(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ char *addr,
	/* in */ long addrlen,
	/* in */ long domain)
{
	return gethostbyaddr(addr,addrlen,domain);
}

/* overridden method ::Sockets::somsGethostbyname */
SOM_Scope Sockets_hostent *SOMLINK tcpsock_somsGethostbyname(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string name)
{
	return gethostbyname(name);
}

/* overridden method ::Sockets::somsGethostent */
SOM_Scope Sockets_hostent *SOMLINK tcpsock_somsGethostent(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev)
{
/*	char buf[256];
	gethostname(buf,sizeof(buf));
	return gethostbyname(buf);*/

	return TCPIPSockets_parent_Sockets_somsGethostent(somSelf,ev);
}

/* overridden method ::Sockets::somsGethostid */
SOM_Scope unsigned long SOMLINK tcpsock_somsGethostid(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev)
{
	char buf[256];
	struct hostent *hp;
	gethostname(buf,sizeof(buf));
	hp=gethostbyname(buf);
	if (hp)
	{
		struct sockaddr_in addr;
		if (sizeof(addr.sin_addr)==hp->h_length)
		{
			memcpy(&addr.sin_addr,hp->h_addr_list[0],hp->h_length);

			return ntohl(addr.sin_addr.s_addr);
		}
	}
	return 0;
}

/* overridden method ::Sockets::somsGethostname */
SOM_Scope long SOMLINK tcpsock_somsGethostname(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string name,
	/* in */ long namelength)
{
	long rc=gethostname(name,namelength);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsGetpeername */
SOM_Scope long SOMLINK tcpsock_somsGetpeername(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	socklen_t nl=*namelen;
	long rc=getpeername(s,name,&nl);
	*namelen=nl;
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsGetservbyname */
SOM_Scope Sockets_servent *SOMLINK tcpsock_somsGetservbyname(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string name,
	/* in */ string protocol)
{
	return getservbyname(name,protocol);
}

/* overridden method ::Sockets::somsGetsockname */
SOM_Scope long SOMLINK tcpsock_somsGetsockname(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	socklen_t nl=*namelen;
	long rc=getsockname(s,name,&nl);
	*namelen=nl;
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsGetsockopt */
SOM_Scope long SOMLINK tcpsock_somsGetsockopt(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long level,
	/* in */ long optname,
	/* in */ char *optval,
	/* out */ long *option)
{
	socklen_t optlen=*option;
	long rc=getsockopt(s,level,optname,optval,&optlen);
	*option=optlen;
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsHtonl */
SOM_Scope unsigned long SOMLINK tcpsock_somsHtonl(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long a)
{
	uinteger4 ai=(uinteger4)a;

	return htonl(ai);
}

/* overridden method ::Sockets::somsHtons */
SOM_Scope unsigned short SOMLINK tcpsock_somsHtons(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short a)
{
	return htons(a);
}

/* overridden method ::Sockets::somsIoctl */
SOM_Scope long SOMLINK tcpsock_somsIoctl(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long cmd,
	/* in */ char *data,
	/* in */ long length)
{
	long rc=
#ifdef _WIN32
	ioctlsocket(s,cmd,(void *)data);
#else
	ioctl(s,cmd,data);
#endif
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsInet_addr */
SOM_Scope unsigned long SOMLINK tcpsock_somsInet_addr(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string cp)
{
	return inet_addr(cp);
}

/* overridden method ::Sockets::somsInet_lnaof */
SOM_Scope unsigned long SOMLINK tcpsock_somsInet_lnaof(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ Sockets_in_addr addr)
{
	return TCPIPSockets_parent_Sockets_somsInet_lnaof(somSelf,ev,addr);
}

/* overridden method ::Sockets::somsInet_makeaddr */
SOM_Scope Sockets_in_addr SOMLINK tcpsock_somsInet_makeaddr(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long net,
	/* in */ unsigned long lna)
{
	return TCPIPSockets_parent_Sockets_somsInet_makeaddr(somSelf,ev,net,lna);
}

/* overridden method ::Sockets::somsInet_netof */
SOM_Scope unsigned long SOMLINK tcpsock_somsInet_netof(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ Sockets_in_addr addr)
{
	unsigned long __result;
	__result=TCPIPSockets_parent_Sockets_somsInet_netof(somSelf,ev,addr);
	return __result;
}

/* overridden method ::Sockets::somsInet_network */
SOM_Scope unsigned long SOMLINK tcpsock_somsInet_network(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string cp)
{
	unsigned long __result;
	__result=TCPIPSockets_parent_Sockets_somsInet_network(somSelf,ev,cp);
	return __result;
}

/* overridden method ::Sockets::somsInet_ntoa */
SOM_Scope string SOMLINK tcpsock_somsInet_ntoa(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ Sockets_in_addr addr)
{
	return inet_ntoa(addr);
}

/* overridden method ::Sockets::somsListen */
SOM_Scope long SOMLINK tcpsock_somsListen(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long backlog)
{
	long rc=listen(s,backlog);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsNtohl */
SOM_Scope unsigned long SOMLINK tcpsock_somsNtohl(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long a)
{
	uinteger4 ai=(uinteger4)a;

	return ntohl(ai);
}

/* overridden method ::Sockets::somsNtohs */
SOM_Scope unsigned short SOMLINK tcpsock_somsNtohs(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short a)
{
	return ntohs(a);
}

/* overridden method ::Sockets::somsReadv */
SOM_Scope long SOMLINK tcpsock_somsReadv(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_iovec *iov,
	/* in */ long iovcnt)
{
	long __result;
	__result=TCPIPSockets_parent_Sockets_somsReadv(somSelf,ev,s,iov,iovcnt);
	return __result;
}
/* overridden method ::Sockets::somsRecv */
SOM_Scope long SOMLINK tcpsock_somsRecv(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ char *buf,
	/* in */ long len,
	/* in */ long flags)
{
	long rc=recv(s,buf,len,flags);
	TCPIP_EXIT_RC(rc);
	return rc;
}
/* overridden method ::Sockets::somsRecvfrom */
SOM_Scope long SOMLINK tcpsock_somsRecvfrom(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ char *buf,
	/* in */ long len,
	/* in */ long flags,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	socklen_t nl=*namelen;
	long rc=recvfrom(s,buf,len,flags,name,&nl);
	*namelen=nl;
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsRecvmsg */
SOM_Scope long SOMLINK tcpsock_somsRecvmsg(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_msghdr *msg,
	/* in */ long flags)
{
	long result=TCPIPSockets_parent_Sockets_somsRecvmsg(somSelf,ev,s,msg,flags);
	TCPIPSockets__set_serrno(somSelf,ev,(result==-1) ? 0 : TCPIPGetLastError());
	return result;
}

/* overridden method ::Sockets::somsSelect */
SOM_Scope long SOMLINK tcpsock_somsSelect(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long nfds,
	/* inout */ Sockets_fd_set *readfds,
	/* inout */ Sockets_fd_set *writefds,
	/* inout */ Sockets_fd_set *exceptfds,
	/* inout */ Sockets_timeval *timeout)
{
	long rc;
#ifdef _WIN32
	/* windows actually ignores the nfds parameters as fd_set contains count
		but complains if no fds are specified */

	nfds=0;

	if (readfds) { nfds+=readfds->fd_count; }
	if (writefds) { nfds+=writefds->fd_count; }
	if (exceptfds) { nfds+=exceptfds->fd_count; }

	if (nfds==0)
	{
		DWORD sv=INFINITE;

		/* no actual sockets so use sleep */

		if (timeout)
		{
			/* 2^10 == 1024 */
			sv=(timeout->tv_sec<<10)+(timeout->tv_usec>>10);
		}

#ifdef _WIN32S
		/* should have a GetMessage/DispatchMessage loop here */

		if (!Sleep(sv))
		{
			return 0;
		}
#else
		/* allow queued user APCs */

		if (!SleepEx(sv,TRUE))
		{
			return 0;
		}
#endif

		return -1;
	}
#endif

	rc=select(nfds,readfds,writefds,exceptfds,timeout);

	TCPIP_EXIT_RC(rc);

	return rc;
}

/* overridden method ::Sockets::somsSend */
SOM_Scope long SOMLINK tcpsock_somsSend(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ char *msg,
	/* in */ long len,
	/* in */ long flags)
{
	long rc=-1;
	CATCH_SIGPIPE_BEGIN
	rc=send(s,msg,len,flags);
	TCPIP_EXIT_RC(rc);
	CATCH_SIGPIPE_END
	return rc;
}

/* overridden method ::Sockets::somsSendmsg */
SOM_Scope long SOMLINK tcpsock_somsSendmsg(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_msghdr *msg,
	/* in */ long flags)
{
	long rc=TCPIPSockets_parent_Sockets_somsSendmsg(somSelf,ev,s,msg,flags);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsSendto */
SOM_Scope long SOMLINK tcpsock_somsSendto(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ char *msg,
	/* in */ long len,
	/* in */ long flags,
	/* inout */ Sockets_sockaddr *to,
	/* in */ long tolen)
{
	long rc=sendto(s,msg,len,flags,to,tolen);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsSetsockopt */
SOM_Scope long SOMLINK tcpsock_somsSetsockopt(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long level,
	/* in */ long optname,
	/* in */ char *optval,
	/* in */ long optlen)
{
	long rc=setsockopt(s,level,optname,optval,optlen);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsShutdown */
SOM_Scope long SOMLINK tcpsock_somsShutdown(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long how)
{
	long rc=shutdown(s,how);
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsSocket */
SOM_Scope long SOMLINK tcpsock_somsSocket(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long domain,
	/* in */ long type,
	/* in */ long protocol)
{
	SOCKET fd=socket(domain,type,protocol);
	TCPIP_EXIT_FD(fd);
	return (long)fd;
}

/* overridden method ::Sockets::somsSoclose */
SOM_Scope long SOMLINK tcpsock_somsSoclose(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s)
{
	long rc=
#ifdef _WIN32
	closesocket(s);
#else
#ifdef __OS2__
	soclose(s);
#else
	close(s);
#endif
#endif
	TCPIP_EXIT_RC(rc);
	return rc;
}

/* overridden method ::Sockets::somsWritev */
SOM_Scope long SOMLINK tcpsock_somsWritev(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_iovec *iov,
	/* in */ long iovcnt)
{
	long __result;
	__result=TCPIPSockets_parent_Sockets_somsWritev(somSelf,ev,s,iov,iovcnt);
	return __result;
}

/* overridden method ::Sockets::_set_serrno */
SOM_Scope void SOMLINK tcpsock__set_serrno(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long serrno)
{
	TCPIPSocketsData *somThis=TCPIPSocketsGetData(somSelf);
	somThis->tcpip_serrno=serrno;
}

/* overridden method ::Sockets::_get_serrno */
SOM_Scope long SOMLINK tcpsock__get_serrno(
	TCPIPSockets SOMSTAR somSelf,
	Environment *ev)
{
	TCPIPSocketsData *somThis=TCPIPSocketsGetData(somSelf);
	return somThis->tcpip_serrno;
}

/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK tcpsock_somInit(
	TCPIPSockets SOMSTAR somSelf)
{
	TCPIPSocketsData *somThis=TCPIPSocketsGetData(somSelf);
#ifdef _WIN32
	WSADATA wsd;
	WSAStartup(0x101,&wsd);
#endif
	somThis->tcpip_serrno=0;
}

/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK tcpsock_somUninit(
	TCPIPSockets SOMSTAR somSelf)
{
#ifdef _WIN32
	WSACleanup();
#endif
}
/* introduced methods for ::TCPIPSockets */



SOMInitModule_begin(somst)
   
	SOMInitModule_new(TCPIPSockets);

SOMInitModule_end

#ifdef _PLATFORM_WIN32_
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
   	{
       	case DLL_PROCESS_ATTACH:
			return DisableThreadLibraryCalls(hInst);
        case DLL_PROCESS_DETACH:
       		return 1;
	}

   	return 1;
}
#endif

SOM_Scope void SOMLINK tcpsock_somDefaultInit(
		TCPIPSockets SOMSTAR somSelf,
		somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	TCPIPSocketsData *somThis;

	TCPIPSockets_BeginInitializer_somDefaultInit

	RHBOPT_unused(somThis);
	tcpsock_somInit(somSelf);

	TCPIPSockets_Init_Sockets_somDefaultInit(somSelf,ctrl);
}

SOM_Scope void SOMLINK tcpsock_somDestruct(
		TCPIPSockets SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	TCPIPSocketsData *somThis;

	TCPIPSockets_BeginDestructor;

	RHBOPT_unused(somThis);
	tcpsock_somUninit(somSelf);

	TCPIPSockets_EndDestructor;
}

