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

#ifndef soms_h
#define soms_h

#ifdef _WIN32
#	if defined(HAVE_WINSOCK2_H) || (_WIN32_WINNT >= 0x400)
#		include <winsock2.h>
#	else
#		include <winsock.h>
#	endif
#else
#	ifdef _PLATFORM_MACINTOSH_
#		define	AF_INET	2
		struct sockaddr { unsigned short sa_family; char sa_data[14]; };
		struct in_addr { unsigned long s_addr; };
		struct sockaddr_in { unsigned  short sin_family; unsigned short sin_port; struct in_addr sin_addr; char sin_zero[8]; };		
#		define FD_SETSIZE  64
		typedef struct { unsigned long fd_count; long fd_array[FD_SETSIZE]; } fd_set;
		struct timeval { unsigned long tv_sec; unsigned long tv_usec; };
		struct  hostent { char *h_name; char **h_aliases; short h_addrtype; short h_length; char **h_addr_list; };
#	else
#		include <sys/types.h>
#		include <sys/time.h>
#		include <sys/socket.h>
#		include <sys/ioctl.h>
#		include <netinet/in.h>
#		include <arpa/inet.h>
#		include <netdb.h>
#	endif
#endif

typedef struct sockaddr Sockets_sockaddr;
typedef struct hostent	Sockets_hostent;
typedef struct servent	Sockets_servent;
typedef struct in_addr	Sockets_in_addr;
typedef struct iovec	Sockets_iovec;
typedef struct msghdr	Sockets_msghdr;
typedef fd_set			Sockets_fd_set;
typedef struct timeval	Sockets_timeval;

#ifdef _PLATFORM_MACINTOSH_
	extern void SOMLINK soms_FD_SET(long,Sockets_fd_set *);
	extern void SOMLINK soms_FD_CLR(long,Sockets_fd_set *);
	extern boolean SOMLINK soms_FD_ISSET(long,Sockets_fd_set *);
#	define FD_ZERO(fds)			(fds)->fd_count=0
#	define FD_SET(fd,fds)		soms_FD_SET(fd,fds)
#	define FD_CLR(fd,fds)		soms_FD_CLR(fd,fds)
#	define FD_ISSET(fd,fds)		soms_FD_ISSET(fd,fds)
#	define htonl(x)		(x)
#	define htons(x)		(x)
#	define ntohs(x)		(x)
#	define ntohl(x)		(x)
#	define INADDR_ANY		0L
#endif

/* Sockets errnos */
#define SOMS_NOERROR			0
#define SOMS_EWOULDBLOCK		1
#define SOMS_EDEADLK			SOMS_EWOULDBLOCK
#define SOMS_EAGAIN				SOMS_EWOULDBLOCK
#define SOMS_EINPROGRESS		2
#define SOMS_EALREADY			3
#define SOMS_ENOTSOCK			4
#define SOMS_EDESTADDRREQ		5
#define SOMS_EMSGSIZE			6
#define SOMS_EPROTOTYPE			7
#define SOMS_ENOPROTOOPT		8
#define SOMS_EPROTONOSUPPORT	9
#define SOMS_ESOCKTNOSUPPORT	10
#define SOMS_EOPNOTSUPP			11
#define SOMS_EPFNOSUPPORT		12
#define SOMS_EAFNOSUPPORT		13
#define SOMS_EADDRINUSE			14
#define SOMS_EADDRNOTAVAIL		15
#define SOMS_ENETDOWN			16
#define SOMS_ENETUNREACH		17
#define SOMS_ENETRESET			18
#define SOMS_ECONNABORTED		19
#define SOMS_ECONNRESET			20
#define SOMS_ENOBUFS			21
#define SOMS_EISCONN			22
#define SOMS_ENOTCONN			23
#define SOMS_ESHUTDOWN			24
#define SOMS_ETOOMANYREFS		25
#define SOMS_ETIMEDOUT			26
#define SOMS_ECONNREFUSED		27
#define SOMS_ELOOP				28
#define SOMS_ENAMETOOLONG		29
#define SOMS_EHOSTDOWN			30
#define SOMS_EHOSTUNREACH		31
#define SOMS_ENOTEMPTY			32
#define SOMS_EBADF				33
#define SOMS_EINTR				34
#define SOMS_EFAULT				35
#define SOMS_EINVAL				36
#define SOMS_EMFILE				37
#define SOMS_ENODEV				38
#define SOMS_UNKNOWNERROR		39
#define SOMS_HOST_NOT_FOUND		40
#define SOMS_TRY_AGAIN			41
#define SOMS_NO_RECOVERY		42
#define SOMS_NO_ADDRESS			43

#ifdef __cplusplus
#	include <somssock.xh>
#else
#	include <somssock.h>
#endif

#endif

