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

#define SOM_Module_somssock_Source
#define Sockets_Class_Source

#include <rhbopt.h>
#include <rhbsomex.h>
#include <somref.h>
#include <somderr.h>
#include <stexcep.h>
#include <snglicls.h>
#include <soms.h>
#include <somssock.ih>

#ifdef _PLATFORM_MACINTOSH_
	#include <Events.h>
	#if GENERATINGCFM
	#else
		#include <LibraryManagerUtilities.h>
	#endif
#else
	#ifdef _WIN32
	#else
		#include <sys/types.h>
		#include <sys/time.h>
		#include <unistd.h>
	#endif
#endif

static void throw_not_implemented(Environment *ev)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,MustOverride,NO);
}


/* overridden methods for ::Sockets */
/* introduced methods for ::Sockets */
/* introduced method ::Sockets::somsAccept */
SOM_Scope long SOMLINK soms_somsAccept(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsBind */
SOM_Scope long SOMLINK soms_somsBind(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_sockaddr *name,
	/* in */ long namelen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsConnect */
SOM_Scope long SOMLINK soms_somsConnect(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_sockaddr *name,
	/* in */ long namelen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsGethostbyaddr */
SOM_Scope Sockets_hostent *SOMLINK soms_somsGethostbyaddr(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ char *addr,
	/* in */ long addrlen,
	/* in */ long domain)
{
	throw_not_implemented(ev);

	return NULL;
}
/* introduced method ::Sockets::somsGethostbyname */
SOM_Scope Sockets_hostent *SOMLINK soms_somsGethostbyname(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string name)
{
	throw_not_implemented(ev);

	return NULL;
}
/* introduced method ::Sockets::somsGethostent */
SOM_Scope Sockets_hostent *SOMLINK soms_somsGethostent(
	Sockets SOMSTAR somSelf,
	Environment *ev)
{
	throw_not_implemented(ev);

	return NULL;
}
/* introduced method ::Sockets::somsGethostid */
SOM_Scope unsigned long SOMLINK soms_somsGethostid(
	Sockets SOMSTAR somSelf,
	Environment *ev)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsGethostname */
SOM_Scope long SOMLINK soms_somsGethostname(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string name,
	/* in */ long namelength)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsGetpeername */
SOM_Scope long SOMLINK soms_somsGetpeername(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsGetservbyname */
SOM_Scope Sockets_servent *SOMLINK soms_somsGetservbyname(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string name,
	/* in */ string protocol)
{
	throw_not_implemented(ev);

	return NULL;
}
/* introduced method ::Sockets::somsGetsockname */
SOM_Scope long SOMLINK soms_somsGetsockname(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsGetsockopt */
SOM_Scope long SOMLINK soms_somsGetsockopt(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long level,
	/* in */ long optname,
	/* in */ char *optval,
	/* out */ long *option)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsHtonl */
SOM_Scope unsigned long SOMLINK soms_somsHtonl(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long a)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsHtons */
SOM_Scope unsigned short SOMLINK soms_somsHtons(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short a)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsIoctl */
SOM_Scope long SOMLINK soms_somsIoctl(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long cmd,
	/* in */ char *data,
	/* in */ long length)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsInet_addr */
SOM_Scope unsigned long SOMLINK soms_somsInet_addr(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string cp)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsInet_lnaof */
SOM_Scope unsigned long SOMLINK soms_somsInet_lnaof(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ Sockets_in_addr addr)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsInet_makeaddr */
SOM_Scope Sockets_in_addr SOMLINK soms_somsInet_makeaddr(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long net,
	/* in */ unsigned long lna)
{
	Sockets_in_addr __result;
	__result.s_addr=0;
	throw_not_implemented(ev);
	return __result;
}
/* introduced method ::Sockets::somsInet_netof */
SOM_Scope unsigned long SOMLINK soms_somsInet_netof(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ Sockets_in_addr addr)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsInet_network */
SOM_Scope unsigned long SOMLINK soms_somsInet_network(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ string cp)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsInet_ntoa */
SOM_Scope string SOMLINK soms_somsInet_ntoa(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ Sockets_in_addr addr)
{
	throw_not_implemented(ev);

	return NULL;
}
/* introduced method ::Sockets::somsListen */
SOM_Scope long SOMLINK soms_somsListen(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long backlog)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsNtohl */
SOM_Scope unsigned long SOMLINK soms_somsNtohl(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long a)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsNtohs */
SOM_Scope unsigned short SOMLINK soms_somsNtohs(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short a)
{
	throw_not_implemented(ev);

	return 0;
}
/* introduced method ::Sockets::somsReadv */
SOM_Scope long SOMLINK soms_somsReadv(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_iovec *iov,
	/* in */ long iovcnt)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsRecv */
SOM_Scope long SOMLINK soms_somsRecv(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ char *buf,
	/* in */ long len,
	/* in */ long flags)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsRecvfrom */
SOM_Scope long SOMLINK soms_somsRecvfrom(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ char *buf,
	/* in */ long len,
	/* in */ long flags,
	/* out */ Sockets_sockaddr *name,
	/* out */ long *namelen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsRecvmsg */
SOM_Scope long SOMLINK soms_somsRecvmsg(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_msghdr *msg,
	/* in */ long flags)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsSelect */
SOM_Scope long SOMLINK soms_somsSelect(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long nfds,
	/* inout */ Sockets_fd_set *readfds,
	/* inout */ Sockets_fd_set *writefds,
	/* inout */ Sockets_fd_set *exceptfds,
	/* inout */ Sockets_timeval *timeout)
{
	throw_not_implemented(ev);

	return -1;
}

/* introduced method ::Sockets::somsSend */
SOM_Scope long SOMLINK soms_somsSend(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ char *msg,
	/* in */ long len,
	/* in */ long flags)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsSendmsg */
SOM_Scope long SOMLINK soms_somsSendmsg(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_msghdr *msg,
	/* in */ long flags)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsSendto */
SOM_Scope long SOMLINK soms_somsSendto(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ char *msg,
	/* in */ long len,
	/* in */ long flags,
	/* inout */ Sockets_sockaddr *to,
	/* in */ long tolen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsSetsockopt */
SOM_Scope long SOMLINK soms_somsSetsockopt(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long level,
	/* in */ long optname,
	/* in */ char *optval,
	/* in */ long optlen)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsShutdown */
SOM_Scope long SOMLINK soms_somsShutdown(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* in */ long how)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsSocket */
SOM_Scope long SOMLINK soms_somsSocket(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long domain,
	/* in */ long type,
	/* in */ long protocol)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsSoclose */
SOM_Scope long SOMLINK soms_somsSoclose(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::somsWritev */
SOM_Scope long SOMLINK soms_somsWritev(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long s,
	/* inout */ Sockets_iovec *iov,
	/* in */ long iovcnt)
{
	throw_not_implemented(ev);

	return -1;
}
/* introduced method ::Sockets::_set_serrno */
SOM_Scope void SOMLINK soms__set_serrno(
	Sockets SOMSTAR somSelf,
	Environment *ev,
	/* in */ long serrno)
{
	throw_not_implemented(ev);
}
/* introduced method ::Sockets::_get_serrno */
SOM_Scope long SOMLINK soms__get_serrno(
	Sockets SOMSTAR somSelf,
	Environment *ev)
{
	throw_not_implemented(ev);

	return -1;
}

#ifdef Sockets_somDefaultInit
SOM_Scope void SOMLINK soms_somDefaultInit(
		Sockets SOMSTAR somSelf,
		somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SocketsData *somThis;

	Sockets_BeginInitializer_somDefaultInit

	Sockets_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
	RHBOPT_unused(somThis);
}
#else
SOM_Scope void SOMLINK soms_somInit(Sockets SOMSTAR somSelf)
{
	Sockets_parent_SOMRefObject_somInit(somSelf);
}
#endif

#ifdef Sockets_somDestruct
SOM_Scope void SOMLINK soms_somDestruct(
		Sockets SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SocketsData *somThis;

	Sockets_BeginDestructor;
	RHBOPT_unused(somThis);

	Sockets_EndDestructor;
}
#else
SOM_Scope void SOMLINK soms_somUninit(Sockets SOMSTAR somSelf)
{
	Sockets_parent_SOMRefObject_somUninit(somSelf);
}
#endif

#ifdef _PLATFORM_MACINTOSH_
void SOMLINK soms_FD_CLR(long fd,Sockets_fd_set *fds)
{
	unsigned long i=0;

	while (i < fds->fd_count)
	{
		if (fds->fd_array[i]==fd)
		{
			fds->fd_count--;
			/* move last one to fill empty slot */

			if (i != fds->fd_count)
			{
				fds->fd_array[i]=fds->fd_array[fds->fd_count];

				break;
			}
		}

		i++;
	}
}

boolean SOMLINK soms_FD_ISSET(long fd,Sockets_fd_set *fds)
{
	if (fds->fd_count)
	{
		long *fdp=fds->fd_array;
		unsigned long i=fds->fd_count;
		while (i--)
		{
			if (fd==*fdp++) return 1;
		}
	}

	return 0;
}

void SOMLINK soms_FD_SET(long fd,Sockets_fd_set *fds)
{
	/* don't add invalid ones */
	if ((fd!=-1)&&(fds))
	{
		/* don't add if already contained.. */
		if (!soms_FD_ISSET(fd,fds))
		{
			fds->fd_array[fds->fd_count++]=fd;
		}
	}
}
#endif

SOMInitModule_begin(soms)

	SOMInitModule_new(Sockets);

SOMInitModule_end


