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
#include <rhbsomd.h>
#include <rhbgiops.h>
#include <somuutil.h>
#include <soms.h>

#define RHBSOMD_EnvironmentInit		{NO_EXCEPTION,{NULL,NULL},NULL}

#ifdef _WIN32
#else
	#ifdef _PLATFORM_UNIX_
		#include <stdio.h>
		#include <fcntl.h>
		#include <signal.h>
		#include <sys/types.h>
		#include <sys/wait.h>
		#include <sys/ioctl.h>
		#include <sys/socket.h>
		#include <sys/time.h>
		#include <errno.h>
		#include <unistd.h>
		#include <netinet/in.h>
		#include <netinet/tcp.h>  /* port to glibc */
	#else
		#define DISABLE_SOCKETS
	#endif
#endif

#ifdef HAVE_STRINGS_H
#	include <strings.h>
#endif

#ifdef DISABLE_SOCKETS
	/* used to disable sockets calls */
	#define close(x)
	#define accept(x,y,z)      		INVALID_SOCKET
	#define	socket(x,y,z)			INVALID_SOCKET
	#define recv(a,b,c,d)			-1
	#define ioctl(a,b,c)			0
	#define bind(x,b,d)		 		-1
	#define connect(x,b,d)			-1
	#define listen(x,y)		 		-1
	#define shutdown(x,y)			-1
	#define send(a,b,c,d)			-1
	#define	getsockname(a,b,c) 		-1
	#define	getpeername(a,b,c)		-1
	#define gethostbyname(a)		NULL
	#define gethostbyaddr(a,b,c)	NULL
	static	int errno=1;
	#define	EWOULDBLOCK				2
	#define	EINPROGRESS				3
	#define	EINTR					4
	#define EADDRINUSE				5
	#define ECONNREFUSED			6
	#define ECONNRESET				7
	#define	ETIMEDOUT				8
	#define	SO_KEEPALIVE			0
	#define inet_addr(x)			0
	static int gethostname(char *p,int len)
	{
		strcpy(p,"localhost");
		return 0;
	}
	#define setsockopt(a,b,c,d,e)	-1
	#define SOL_SOCKET   0
	#define SO_REUSEADDR	0
#else
	#ifdef USE_THREADS
	#else
		#define close(x)				use SOM Sockets
		#define soclose(x)				use SOM Sockets
		#define closesocket(x)			use SOM Sockets
		#define ioctlsocket(x)			use SOM Sockets
		#define accept(x,y,z)      		use SOM Sockets
		#define	socket(x,y,z)			use SOM Sockets
		#define recv(a,b,c,d)			use SOM Sockets
		#define ioctl(a,b,c)			use SOM Sockets
		#define bind(x,b,d)		 		use SOM Sockets
		#define connect(x,b,d)			use SOM Sockets
		#define listen(x,y)		 		use SOM Sockets
		#define shutdown(x,y)			use SOM Sockets
		#define send(a,b,c,d)			use SOM Sockets
		#define	getsockname(a,b,c) 		use SOM Sockets
		#define	getpeername(a,b,c)		use SOM Sockets
		#define gethostbyname(a)		use SOM Sockets
		#define gethostbyaddr(a,b,c)	use SOM Sockets
		#ifdef errno
			#undef errno
		#endif
		#define errno					use SOM Sockets
		#ifdef inet_addr
			#undef inet_addr
		#endif
		#define inet_addr(x)			use SOM Sockets
		#define gethostname				use SOM Sockets
		#define setsockopt(a,b,c,d,e)	use SOM Sockets
		#ifdef WSAGetLastError
			#undef WSAGetLastError
		#endif
		#define WSAGetLastError			use SOM Sockets
		#ifdef EWOULDBLOCK	
			#undef EWOULDBLOCK
		#endif
		#ifdef EINPROGRESS	
			#undef EINPROGRESS
		#endif
		#ifdef WSAEWOULDBLOCK	
			#undef WSAEWOULDBLOCK
		#endif
		#ifdef WSAEINPROGRESS	
			#undef WSAEINPROGRESS
		#endif
	#endif
#endif

#ifdef USE_THREADS
#	include <somserr.h>
#endif

#define RHBSOCK_STATE_IDLE			0	/* nothing */
#define RHBSOCK_STATE_PASSIVE		1	/* listening */
#define RHBSOCK_STATE_ACTIVE		2	/* connecting */
#define RHBSOCK_STATE_UP			3	/* connected */
#define RHBSOCK_STATE_BROKEN		4	/* broken */
#define RHBSOCK_STATE_CLOSED		5	/* received zero bytes */
#define RHBSOCK_STATE_DELETING		6	/* deleting.. */

#define IS_DELETING(x)		((x)->state==RHBSOCK_STATE_DELETING)
#define IS_CONNECTED(x)		((x)->state==RHBSOCK_STATE_UP)
#define IS_LISTENER(x)		((x)->state==RHBSOCK_STATE_PASSIVE)
#define IS_CONNECTING(x)	((x)->state==RHBSOCK_STATE_ACTIVE)

#ifdef _DEBUGx
	#define somPrintf  dont use
	#define printf  dont use
#endif

static boolean rhbbsd_IsJustIPV4Address(const char *p)
{
	while (*p)
	{
		char c=*p++;

		if (c != '.')
		{
			if ((c < '0')||(c > '9'))
			{
				return 0;
			}
		}
	}

	return 1;
}

#if defined(HAVE_GETADDRINFO) && defined(USE_THREADS)
static int do_include(const struct addrinfo *addr)
{
	switch (addr->ai_family)
	{
		case AF_INET:
#ifdef AF_INET6
		case AF_INET6:
#endif
			return 1;
	}

	return 0;
}
#endif

#if (!defined(HAVE_GETADDRINFO) && !defined(USE_RESOLVER_CHILD)) || !defined(USE_THREADS)
static void build_addrSeq(_IDL_SEQUENCE_SOMD_NetworkAddress *addr,struct hostent *hp,unsigned short port)
{
	char **h=hp->h_addr_list;
	if (h)
	{
		while (*h)
		{
			h++;
			addr->_maximum++;
		}
	}
	if (addr->_maximum)
	{
		size_t k=sizeof(addr->_buffer[0])*addr->_maximum;
		SOMD_NetworkAddress *ap=addr->_buffer=SOMMalloc(k);
		memset(addr->_buffer,0,k);
		h=hp->h_addr_list;
		while (*h)
		{
			ap->name=NULL;
			ap->family=hp->h_addrtype;

			switch (ap->family)
			{
			case AF_INET:
				if (hp->h_length==4)
				{
					struct sockaddr_in *sin4=SOMMalloc(sizeof(sin4[0]));
					ap->value._buffer=(octet *)sin4;
					ap->value._length=sizeof(sin4[0]);
					ap->value._maximum=sizeof(sin4[0]);
					sin4->sin_family=AF_INET;
#ifdef HAVE_SOCKADDR_SA_LEN
					sin4->sin_len=sizeof(sin4[0]);
#endif
					sin4->sin_port=htons(port);
					memcpy(&(sin4->sin_addr),*h,hp->h_length);
					ap++;
					addr->_length++;
				}
				break;
#ifdef HAVE_SOCKADDR_IN6
			case AF_INET6:
				if (hp->h_length==16)
				{
					struct sockaddr_in6 *sin6=SOMMalloc(sizeof(sin6[0]));
					ap->value._buffer=(octet *)sin6;
					ap->value._length=sizeof(sin6[0]);
					ap->value._maximum=sizeof(sin6[0]);
					sin6->sin6_family=AF_INET6;
#ifdef HAVE_SOCKADDR_SA_LEN
					sin6->sin6_len=sizeof(sin6[0]);
#endif
					sin6->sin6_port=htons(port);
					memcpy(&(sin6->sin6_addr),*h,hp->h_length);
					ap++;
					addr->_length++;
				}
				break;
#endif
			}

			h++;
		}

	}
}
#endif

#if defined(_DEBUG) && defined(USE_THREADS) && 0
static void SOM_dump_errno(char *why,int err,char *file,int line)
{
	char *p;
	char errstr[32];

	switch (err)
	{
#ifdef _WIN32
	case WSAEWOULDBLOCK: p="WSAEWOULDBLOCK"; break;
	case WSAEINPROGRESS: p="WSAEINPROGRESS"; break;
	case WSAECONNRESET: p="WSAECONNRESET"; break;
	case WSAEINTR: p="WSAEINTR"; break;
	case WSAECONNREFUSED: p="WSAECONNREFUSED"; break;
	case WSAEADDRINUSE: p="WSAEADDRINUSE"; break;
	case WSAETIMEDOUT: p="WSAETIMEDOUT"; break;
#else
	case EWOULDBLOCK: p="EWOULDBLOCK"; break;
	case EINPROGRESS: p="EINPROGRESS"; break;
	case ECONNRESET: p="ECONNRESET"; break;
	case ECONNREFUSED: p="ECONNREFUSED"; break;
	case EINTR: p="EINTR"; break;
	case EADDRINUSE: p="EADDRINUSE"; break;
	case ETIMEDOUT: p="ETIMEDOUT"; break;
#endif
	default:
#ifdef HAVE_SNPRINTF
		snprintf(errstr,sizeof(errstr),"%d",err);
#else
		sprintf(errstr,"%d",err);
#endif
		p=errstr;
	}

	debug_somPrintf(("%s errno=%s at %s:%d\n",why,p,file,line));

	RHBOPT_unused(why)
	RHBOPT_unused(line)
	RHBOPT_unused(file)
}
#endif

#define SOM_perrno(a,b)   SOM_dump_errno(a,b,__FILE__,__LINE__);

#if defined(_DEBUG) && defined(_WIN32)
static void validate_address(RHBSocketAddress *addr,int len)
{
	switch (addr->sa_family)
	{
	case AF_INET:
		RHBOPT_ASSERT(len==sizeof(struct sockaddr_in))
		break;
#ifdef HAVE_SOCKADDR_IN6
	case AF_INET6:
		RHBOPT_ASSERT(len==sizeof(struct sockaddr_in6))
		break;
#endif
	default:
		RHBOPT_ASSERT(&addr)
		break;
	}
}
#define VALIDATE_ADDRESS(x,y)   validate_address(x,y);
#else
	#define VALIDATE_ADDRESS(x,y)
#endif

#if defined(USE_THREADS) && !defined(__OS2__)
static int soclose(SOCKET fd)
{
#ifdef DEBUG_soclose
	RHBORB_sockaddr addr;
	int j=sizeof(addr);
	int i=getsockname(fd,&addr.u.generic,&j);
	if (!i)
	{
		switch (addr.u.generic.sa_family)
		{
		case AF_INET:
			printf("soclose(%s:%d)\n",
					inet_ntoa(addr.u.ipv4.sin_addr),ntohs(addr.u.ipv4.sin_port));
			break;
#ifdef HAVE_SOCKADDR_IN6
		case AF_INET6:
			{
				char buf[256];
			printf("soclose(%s:%d)\n",
					inet_ntop(AF_INET6,&addr.u.ipv6.sin6_addr,buf,sizeof(buf)),ntohs(addr.u.ipv6.sin6_port));
			}
			break;
#endif
		}
	}
#else
	int 
#endif
#ifdef _WIN32
	i=closesocket(fd);
	RHBOPT_ASSERT(!i)
#else
	i=close(fd);
#ifdef _DEBUG
	if (i) 
	{ 
		perror("soclose()");

#ifdef DEBUGGING_ON_TRU64_JUST_TO_SEE
		/* Tru64 5.1 was returning an error particularly when
		   the socket previously failed to connect, but
		   a following close returned EBADF so the previous
           close had actually worked so dangerous to try again */
		if (i) 
		{ 
			i=close(fd);
			if (i)
			{
				perror("soclose2()"); 
			}
		}
#endif
	}
#endif
/*	RHBOPT_ASSERT(!i) */
#endif
	return i;
}
#endif /* __OS2__ */

#if 0
	#define somPrintf not to be used
	#undef SOMD_bomb
	#define SOMD_bomb(x)
	#undef debug_somPrintf
	#define debug_somPrintf(x)
#endif

#if (defined(_WIN32)) && defined(USE_THREADS) 
#define socketpair win32_socketpair
static int __stdcall socketpair(int family,int type,int protocol,SOCKET *result)
{
	SOCKET a=INVALID_SOCKET,b=INVALID_SOCKET,c=INVALID_SOCKET;
	int rc=-1;

#if defined(USE_THREADS)
	RHBOPT_ASSERT(family != AF_UNIX)
#ifdef AF_INET6
	RHBOPT_ASSERT((family==AF_INET)||(family==AF_INET6))
#else
	RHBOPT_ASSERT(family==AF_INET)
#endif
#else
	RHBOPT_ASSERT(family==AF_UNIX);
	family=AF_INET;
#endif

	RHBOPT_ASSERT(type == SOCK_STREAM)

	result[0]=INVALID_SOCKET;
	result[1]=INVALID_SOCKET;

	__try
	{
		a=socket(family,type,0);
		b=socket(family,type,0);

		if ((a!=INVALID_SOCKET) && (b!=INVALID_SOCKET))
		{
			union
			{
				struct sockaddr addr;
				struct sockaddr_in addr_in;
#ifdef HAVE_SOCKADDR_IN6
				struct sockaddr_in6 addr_in6;
#endif
			} addr;

			memset(&addr,0,sizeof(addr));

			addr.addr_in.sin_family=(short)family;

			switch (family)
			{
			case AF_INET:
				addr.addr_in.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
				break;
#ifdef HAVE_SOCKADDR_IN6
			case AF_INET6:
				{
					static struct 
#ifdef HAVE_IN6_ADDR
							in6_addr 
#else
							in_addr6
#endif
							a6={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}};
					addr.addr_in6.sin6_addr=a6;
				}
				break;
#endif
			}

			if (!bind(a,&addr.addr,sizeof(addr)))
			{
				if (!listen(a,1))
				{
					socklen_t j=sizeof(addr);

					if (!getsockname(a,&addr.addr,&j))
					{
						RHBOPT_ASSERT(addr.addr.sa_family==family)

						if (!connect(b,&addr.addr,j))
						{
							c=accept(a,&addr.addr,&j);

							if (c!=INVALID_SOCKET) 
							{
								rc=0;

								result[0]=b;
								result[1]=c;
								b=INVALID_SOCKET;
								c=INVALID_SOCKET;
							}
						}
					}
				}
			}
		}
	}
	__finally
	{
		if (a!=INVALID_SOCKET) closesocket(a);
		if (b!=INVALID_SOCKET) closesocket(b);
		if (c!=INVALID_SOCKET) closesocket(c);
	}

	return rc;
}
#endif

#ifndef INADDR_NONE
	#define INADDR_NONE   ((in_addr_t)~0L)
#endif

RHBSocket_impl(rhbbsd)
RHBProtocol_impl(rhbbsdp)
RHBResolver_impl(rhbbsdnr)

#ifdef USE_THREADS
static void SOMLINK rhbbsdr_thread(void *a,Environment *ev);

struct rhbbsd_thread_selector;

#define RHBSocket_select_read		1
#define RHBSocket_select_write		2
#define RHBSocket_select_except		4
#define RHBSocket_select_remove		8
#define RHBSocket_select_connect	16

struct thread_task
{
	struct thread_select *task_thread; /* back to zero when removed */
	struct thread_task *task_next;
	SOCKET fd;
	int selectFlags; /* 1=read/2=write/4=except,8==remove */
	void (*notify)(struct rhbbsd_thread_selector *,struct thread_task *,int);
	struct RHBSocketData *data;
	void (*removed_locked)(struct thread_task *);
};

struct thread_select
{
	RHBProtocol *protocol;
	struct thread_select *next;
	SOCKET fd_signal_write;
	SOCKET fd_signal_read;
	int family;
	struct thread_task *tasks;
	int numTasks;
	boolean running;
	fd_set *fdr;
	fd_set *fdw;
	fd_set *fde;
	int blip_write,blip_read;
#ifdef USE_PTHREADS
	pthread_t tid;
#else
	DWORD tid;
#endif
	struct RHBORB_thread_task task;
};

static boolean thread_match(struct thread_select *somThis)
{
	if (somThis)
	{
		/* don't know the tid until we're running */
		if (somThis->running)
		{
			return (boolean)(
#ifdef USE_PTHREADS
			pthread_equal(pthread_self(),somThis->tid)
#else
			somThis->tid==GetCurrentThreadId()
#endif
			);
		}
	}

	return 0;
}

static boolean thread_task_detach(struct thread_task *task)
{
	struct thread_select *thread=task->task_thread;

	if (thread)
	{
		/* if the select thread hasn't started running,
			or we're actually in the same thread,
			this is safe to do.. */

		if ((!thread->running) || thread_match(thread))
		{
			if (task->fd != INVALID_SOCKET)
			{
				/* don't want to get noticed as part of
					fd_sets in case once this is closed
					the file descriptor is used again,
				
				  this is only valid to do while not actually
				  executing the select(), hence the ptrs get
				  reset
				*/

				if (thread->fdr) FD_CLR(task->fd,thread->fdr);
				if (thread->fdw) FD_CLR(task->fd,thread->fdw);
				if (thread->fde) FD_CLR(task->fd,thread->fde);
			}

			task->task_thread=NULL;

			if (thread->tasks==task)
			{
				thread->tasks=task->task_next;
				task->task_next=NULL;
			}
			else
			{
				struct thread_task *p=thread->tasks;

				while (p)
				{
					if (p->task_next==task)
					{
						RHBOPT_ASSERT(p != task)

						p->task_next=task->task_next;
						task->task_next=NULL;

						break;
					}
					else
					{
						p=p->task_next;
					}
				}
			}

			return 1;
		}

		/* can't detach from the wrong thread,
		as may still be in the select statement */

		return 0;
	}

	return 1;
}

static void thread_blip(struct thread_select *somThis)
{
	/* only blip the thread from a different thread */

	RHBOPT_ASSERT(somThis)

	if (somThis)
	{
		if (somThis->blip_write==somThis->blip_read)
		{
			if (!thread_match(somThis))
			{
				if (somThis->fd_signal_write!=INVALID_SOCKET)
				{
					int i=
#if defined(_WIN32) || defined(__OS2__)
						send(somThis->fd_signal_write,"\0",1,0);
#else
						write(somThis->fd_signal_write,"\0",1);
#endif

					RHBOPT_ASSERT(i > 0)

					if (i > 0)
					{
						somThis->blip_write+=i;
					}
			/*		else
					{
						SOMD_bomb("blipping failed");
					}*/
				}
			}
		}
	}
}
#endif

static void close_on_exec(SOCKET fd)
{
	/* don't want new processes inheriting any of these
		sockets */
#ifndef _WIN32
	#ifdef F_SETFD
		#ifdef FD_CLOEXEC
			int i=fcntl(fd,F_SETFD,FD_CLOEXEC);
		#else
			int i=fcntl(fd,F_SETFD,1);
		#endif
		if (i)
		{
			debug_somPrintf(("Did not set F_SETFD(%d)\n",errno));
		}
	#else
		int b=1;
		int i=ioctl(fd,FIOCLEX,(char *)&b);

		if (i)
		{
			debug_somPrintf(("Did not set FIOCLEX(%d)\n",errno));
		}
	#endif
#endif
}

static int set_non_blocking(
#if !defined(USE_THREADS)
		Sockets SOMSTAR socketObject,
#endif
		SOCKET fd)
{
#ifdef USE_THREADS
	#ifdef _WIN32
		BOOL ul=1;
		int i=ioctlsocket(fd,FIONBIO,(unsigned long *)&ul);
		RHBOPT_ASSERT(!i);
		return i;
	#else
		#ifdef HAVE_FCNTL_F_SETFL_O_NDELAY
			return fcntl(fd,F_SETFL,O_NDELAY);
		#else
			int ul=1;
			return ioctl(fd,FIONBIO,(void *)&ul);
		#endif
	#endif
#else
	Environment ev=RHBSOMD_EnvironmentInit;
	int ul=1;
	return Sockets_somsIoctl(socketObject,&ev,fd,FIONBIO,(char *)&ul,sizeof(ul));
#endif
}

static RHBSocketRef rhbbsd_create_from_fd(RHBProtocolRef protocol,SOCKET fd,int family);

typedef struct RHBSocketData
{
	RHBSocket *sock;
	RHBProtocolRef protocol;
	SOCKET fd;
	union
	{
		struct sockaddr abstract;
		struct sockaddr_in in;
#ifdef HAVE_SOCKADDR_IN6
		struct sockaddr_in6 in6;
#endif
	} remote_addr,local_addr;
	RHBSocketLength remote_addrlen,local_addrlen;
	rhbatomic_t lUsage;
	unsigned char state;
	RHBSocketSink *sink;
	boolean writes_wanted;

#ifdef USE_THREADS
	struct thread_task task;
	#ifdef USE_PTHREADS
		pthread_cond_t *kick_on_close;
	#else
		HANDLE kick_on_close;
	#endif
#else
	RHBSocketEvent *event_token;
	Sockets SOMSTAR socketObject;
#endif
	struct RHBSocketData *next;
} RHBSocketData; 

struct RHBProtocolData
{
	rhbatomic_t lUsage;
	/* nothing specific for threaded apps */
	RHBORB *orb;
	boolean closing;
	RHBSocketData *socket_impl_list;
	struct RHBResolverData *resolvers;
#ifdef USE_THREADS
	struct thread_select *threads;
#else
	Sockets SOMSTAR socketObject;
#endif
};

struct RHBResolverData
{
	char *name;
	struct RHBResolverData *next;
	RHBResolver_Callback cb;
	RHBImplementationDef *impl;
	RHBProtocol *protocol;
	RHBResolver *resolver;
	rhbatomic_t lUsage;
	unsigned short port;

#ifdef USE_RESOLVER_CHILD
	_IDL_SEQUENCE_octet dataSeq;
	SOCKET fd;
	char length_bytes[4];
	int length_bytes_len;
	unsigned long packet_size;
#ifdef _WIN32
	void *hThread;
#else
	int pid;
#endif
#endif

#ifdef USE_THREADS
	struct RHBORB_thread_task task;
#endif
};

typedef struct RHBResolverData RHBResolverData;

#define RHBResolverGetData(x)   x->impl

#define DEBUG_HERE			,__FILE__,__LINE__

#ifdef USE_THREADS
static void SOMLINK rhbbsd_thread_selector_final(void *pv)
{
struct thread_select *somThis=pv;

	somThis->running=0;

	if (somThis->protocol)
	{
		if (somThis==somThis->protocol->impl->threads)
		{
			somThis->protocol->impl->threads=somThis->next;
		}
		else
		{
			struct thread_select *p=somThis->protocol->impl->threads;

			while (p)
			{
				if (p->next==somThis)
				{
					p->next=somThis->next;
					break;
				}
				else
				{
					p=p->next;
				}
			}
		}
	}

	while (somThis->tasks)
	{
		struct thread_task *task=somThis->tasks;

		somThis->tasks=task->task_next;

		task->task_next=NULL;

		if (task->task_thread==somThis)
		{
			task->task_thread=NULL;

			task->removed_locked(task);
		}
	}

	soclose(somThis->fd_signal_write);
	soclose(somThis->fd_signal_read);
	SOMFree(somThis);
}

struct rhbbsd_thread_selector
{
	boolean locked;
	struct thread_select *thread;
#if defined(USE_PTHREADS)
	sigset_t old_sigmask;
#endif
};

RHBOPT_cleanup_begin(rhbbsd_thread_selector_cleanup,pv)

struct rhbbsd_thread_selector *data=pv;

	if (!data->locked)
	{
		RHBORB_guard(0);

		data->locked=1;
	}

	rhbbsd_thread_selector_final(data->thread);

#ifdef USE_PTHREADS
	pthread_sigmask(SIG_SETMASK,&data->old_sigmask,NULL);
#endif

	data->locked=0;

	RHBORB_unguard(0)

RHBOPT_cleanup_end

static void SOMLINK rhbbsd_thread_selector(void *pv,Environment *ev)
{
struct rhbbsd_thread_selector data={0,NULL};
struct thread_select *somThis=pv;
#if defined(USE_PTHREADS)
	sigset_t new_sigmask;
/* don't want any of these signals messing up the sockets in
	the thread that does all the work */

	sigemptyset(&new_sigmask);
#ifdef SIGIO
	sigaddset(&new_sigmask,SIGIO);
#endif
#ifdef SIGPIPE
	sigaddset(&new_sigmask,SIGPIPE);
#endif
#ifdef USE_PTHREADS
	pthread_sigmask(SIG_BLOCK,&new_sigmask,&data.old_sigmask);
#else
	pth_sigmask(SIG_BLOCK,&new_sigmask,&data.old_sigmask);
#endif
#endif

	data.thread=somThis;

	RHBORB_guard(0)

	data.locked=1;

#ifdef USE_PTHREADS
	somThis->tid=pthread_self();
#else
	somThis->tid=GetCurrentThreadId();
#endif

	somThis->running=1;

	RHBOPT_cleanup_push(rhbbsd_thread_selector_cleanup,&data);

	while (somThis->tasks)
	{
		struct thread_task *task=somThis->tasks;
		struct thread_task *q=NULL;

		while (task)
		{
			if (task->selectFlags & RHBSocket_select_connect)
			{
				break;
			}

			if (task->selectFlags & RHBSocket_select_remove)
			{
				task->task_thread=NULL;

				if (q)
				{
					RHBOPT_ASSERT( q!=task )

					q->task_next=task->task_next;
				}
				else
				{
					somThis->tasks=task->task_next;
				}

				task->task_next=NULL;

				break;
			}
			else
			{
				q=task;
				task=task->task_next;
			}
		}

		if (task)
		{
			if (task->selectFlags & RHBSocket_select_connect)
			{
				task->notify(&data,task,RHBSocket_select_connect);
			}
			else
			{
				task->removed_locked(task);
			}
		}
		else
		{
			fd_set fdr,fdw,fde;
			SOCKET n=somThis->fd_signal_read;
#ifdef _DEBUG_SELECT
			int n_count=0;
			struct timeval ts={0,0};
#	define RHBSOMD_select_timeout  &ts
#else
#	define RHBSOMD_select_timeout  NULL
#endif
			int i=0;
			FD_ZERO(&fdr);
			FD_ZERO(&fdw);
			FD_ZERO(&fde);

			if (somThis->fd_signal_read!=INVALID_SOCKET)
			{
				FD_SET(somThis->fd_signal_read,&fdr);
			}

#ifdef _DEBUG_SELECT
			ts.tv_sec=10;
#endif

			task=somThis->tasks;

			while (task)
			{
				if (task->selectFlags & 
						(RHBSocket_select_read|
						RHBSocket_select_write|
						RHBSocket_select_except))
				{
					if (task->fd!=INVALID_SOCKET)
					{
						if (task->fd > n) n=task->fd;

						if (task->selectFlags & RHBSocket_select_read) FD_SET(task->fd,&fdr);
						if (task->selectFlags & RHBSocket_select_write) FD_SET(task->fd,&fdw);
						if (task->selectFlags & RHBSocket_select_except) FD_SET(task->fd,&fde);

			/*			debug_somPrintf(("select(%d,%c%c%c)\n",
								task->fd,
								(task->selFlags & 4 ) ? 'e' : '-',
								(task->selFlags & 2 ) ? 'w' : '-',
								(task->selFlags & 1 ) ? 'r' : '-'
								)); */

#ifdef _DEBUG_SELECT
						n_count++;
#endif
					}
				}

				task=task->task_next;
			}

			data.locked=0;

			RHBORB_unguard(0)

			i=select((int)(n+1),&fdr,&fdw,&fde,RHBSOMD_select_timeout);

			RHBORB_guard(0)

			data.locked=1;

			if (i > 0)
			{
				if (somThis->fd_signal_read!=INVALID_SOCKET)
				{
					if (FD_ISSET(somThis->fd_signal_read,&fdr))
					{
						char buf[256];
						int j;

						FD_CLR(somThis->fd_signal_read,&fdr);
#	if defined(_WIN32) || defined(__OS2__)
						j=recv(somThis->fd_signal_read,buf,sizeof(buf),0);
#	else
						j=read(somThis->fd_signal_read,buf,sizeof(buf));
#	endif

						if (j==0)
						{
							break;
						}

						if (j > 0)
						{
							somThis->blip_read+=j;
						}

						i--;
					}
				}

				if (i > 0)
				{
					somThis->fdr=&fdr;
					somThis->fdw=&fdw;
					somThis->fde=&fde;
					while (i > 0)
					{
						int flags=0;

						task=somThis->tasks;

						while (task)
						{
							if ((task->selectFlags & RHBSocket_select_read)&&(FD_ISSET(task->fd,&fdr)))
							{
								FD_CLR(task->fd,&fdr);

								flags|=RHBSocket_select_read;
							}

							if ((task->selectFlags & RHBSocket_select_write)&&(FD_ISSET(task->fd,&fdw)))
							{
								FD_CLR(task->fd,&fdw);

								flags|=RHBSocket_select_write;
							}

							if ((task->selectFlags & RHBSocket_select_except)&&(FD_ISSET(task->fd,&fde)))
							{
								FD_CLR(task->fd,&fde);

								flags|=RHBSocket_select_except;
							}

							if (flags) break;

							task=task->task_next;
						}

						if (task)
						{
							task->notify(&data,task,flags);
						}
						else
						{
							break;
						}
					}

					somThis->fdr=NULL;
					somThis->fdw=NULL;
					somThis->fde=NULL;
				}
			}
			else
			{
				if (i < 0)
				{
#ifdef _DEBUG
    #ifdef _WIN32
                    int e=WSAGetLastError();
    #else
                    int e=errno;
    #endif
                    somPrintf("errno=%d\n",e);
                    RHBOPT_ASSERT(!i)


					debug_somPrintf(("select error %d\n",e));
#endif
					break;
				}
				else
				{
#ifdef _DEBUG_SELECT
#	ifdef _WIN32
					somPrintf("# %ld:select(%d)\n",GetCurrentThreadId(),n_count);
#	else
					somPrintf("# select(%d)\n",n_count);
#	endif
#endif
				}
			}
		}
	}

	RHBOPT_cleanup_pop();
}

static struct thread_select *rhbbsdp_get_thread(RHBProtocol *proto,struct thread_task *task,int family)
{
	struct thread_select *t=proto->impl->threads;

	task->task_next=NULL;

	while (t)
	{
		long i=0;
		struct thread_task *p=t->tasks;
#ifdef _WIN32
		fd_set fds;
		fds.fd_count=0;
#endif

#ifdef _WIN32
	#define	NUMBER_TASKS_PER_THREAD		((sizeof(fds.fd_array)/sizeof(fds.fd_array[0]))-2)
#else
	#define NUMBER_TASKS_PER_THREAD		((sizeof(fd_set)<<3)-5)
#endif

		if (t->family != family) 
		{
			i=NUMBER_TASKS_PER_THREAD;
		}
		else
		{
			while (p)
			{
				RHBOPT_ASSERT(p != p->task_next)

				i++;

				p=p->task_next;
			}
		}


		if (i < NUMBER_TASKS_PER_THREAD)
		{
			RHBOPT_ASSERT(t->tasks != task)

			task->task_next=t->tasks;
			t->tasks=task;
			task->task_thread=t;
			thread_blip(t);

			break;
		}
		else
		{
			t=t->next;
		}
	}

	if (!t)
	{
		SOCKET fds[2]={INVALID_SOCKET,INVALID_SOCKET};
		if (!
#ifdef _WIN32
			socketpair(family,SOCK_STREAM,0,fds)
#else
			pipe(fds)
#endif
		)
		{
			/* don't want sticky fingers on this pipe... */
			close_on_exec(fds[0]);
			close_on_exec(fds[1]);

			t=SOMMalloc(sizeof(t[0]));

			t->next=proto->impl->threads;
			proto->impl->threads=t;
			t->tasks=task;

			t->fd_signal_read=fds[0];
			t->fd_signal_write=fds[1];
			t->family=family;

			t->numTasks=0;
			t->running=0;
			t->protocol=proto;
			task->task_thread=t;
			t->fdr=t->fde=t->fdw=NULL;
			t->blip_write=t->blip_read=0;

			set_non_blocking(t->fd_signal_read);
			set_non_blocking(t->fd_signal_write);

			t->task.start=rhbbsd_thread_selector;
			t->task.param=t;
			t->task.failed=rhbbsd_thread_selector_final;
			RHBORB_StartThreadTask(proto->impl->orb,&t->task);
		}
	}

	return t;
}

#endif

static int RHBLINK rhbbsd_AddRef(RHBSocket * somSelf)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);

	somd_atomic_inc(&somThis->lUsage);

	return 0;
}

static int RHBLINK rhbbsd_Release(RHBSocket * somSelf)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);
RHBSocketData *others;

	if (somd_atomic_dec(&somThis->lUsage))
	{
		return 0;
	}

	RHBORB_guard(0)

	if (somThis->lUsage) 
	{
		RHBORB_unguard(0)

		return 0;
	}

	if (IS_DELETING(somThis)) 
	{
		RHBORB_unguard(0)

		return 0;
	}

#ifdef USE_THREADS
	if (somThis->task.task_thread)
	{
		somd_atomic_inc(&somThis->lUsage);

		somThis->task.selectFlags=RHBSocket_select_remove;

		thread_blip(somThis->task.task_thread);

		RHBORB_unguard(0)

		return 0;
	}
#endif

	somThis->state=RHBSOCK_STATE_DELETING;

	if (somThis->fd!=INVALID_SOCKET)
	{
#ifdef USE_THREADS
#else
		Environment ev=RHBSOMD_EnvironmentInit;
#endif
		SOCKET fd=somThis->fd;

		somThis->fd=INVALID_SOCKET;

#ifdef USE_THREADS
		soclose(fd);
#else
		Sockets_somsSoclose(somThis->socketObject,&ev,fd);
#endif
	}

#ifdef USE_THREADS
	if (somThis->lUsage)
	{
		RHBSOM_Trace("usage was upped during RHBSocket_Release")
		RHBORB_unguard(0)
		return 0;
	}
#endif

	others=somThis->protocol->impl->socket_impl_list;

	if (others==somThis)
	{
		somThis->protocol->impl->socket_impl_list=somThis->next;
	}
	else
	{
		while (others->next != somThis)
		{
			others=others->next;
		}

		others->next=somThis->next;
	}

	RHBORB_unguard(somThis->orb);

#ifdef USE_SELECT
	if (somThis->event_token)
	{
		RHBORB_remove_socket_event(somThis->protocol->impl->orb,somThis->event_token);

		somThis->event_token=0;
	}
#endif

	RHBOPT_ASSERT(!somThis->lUsage) 

	if (somThis->lUsage)
	{
		return 0;
	}

	somThis->protocol->vtbl->Release(somThis->protocol);

	SOMFree(somSelf);

	return 0;
}

static int RHBLINK rhbbsd_QueryInterface(RHBSocket *somSelf,void *refidd,void **ppv)
{
/*RHBSocketData *somThis=RHBSocketGetData(somSelf);*/

	RHBOPT_unused(somSelf)
	RHBOPT_unused(ppv)
	RHBOPT_unused(refidd)

	return -1;
}

static int RHBLINK rhbbsd_RecvData(RHBSocket *somSelf,RHBSocketError *ev,void *data,int len)
{
#ifndef USE_THREADS
Environment ev2=RHBSOMD_EnvironmentInit;
#endif
RHBSocketData *somThis=RHBSocketGetData(somSelf);
int i,err=0;

	if ((somThis->fd == INVALID_SOCKET) || (!IS_CONNECTED(somThis)))
	{
/*		somPrintf("bad recv...\n");*/

		*ev=SOMS_ENOTSOCK;

		return -1;
	}

	RHBSOM_Trace("recv start")

#ifdef USE_THREADS
	i=recv(somThis->fd,data,len,0);
#else
	RHBOPT_ASSERT(somThis->socketObject);
	i=Sockets_somsRecv(somThis->socketObject,&ev2,somThis->fd,data,len,0);
#endif

	if (i==-1)
	{
#ifdef USE_THREADS
		err=Sockets_GetError(
	#ifdef _WIN32
			WSAGetLastError()
	#else
			errno
	#endif
			);
#else
		err=Sockets__get_serrno(somThis->socketObject,&ev2);
#endif
	}

	RHBSOM_Trace("recv end")

	if (i != -1) 
	{
		if (!i)
		{
			somThis->state=RHBSOCK_STATE_CLOSED;
		}
	}
	else
	{
		*ev=err;

		if (somThis->fd==INVALID_SOCKET)
		{
			/* no more to read */
			i=0;
		}
		else
		{
			switch (err)
			{
			case SOMS_EWOULDBLOCK:
				break;
			default:
				somThis->state=RHBSOCK_STATE_BROKEN;
				break;
			}
		}
	}

#ifndef USE_THREADS
	SOM_UninitEnvironment(&ev2);
#endif

	return i;
}

#if defined(SIGPIPE)
	#define CATCH_SIGPIPE
	static void catch_sigpipe(int i) { }
#endif

static int RHBLINK rhbbsd_SendData(RHBSocket *somSelf,RHBSocketError *ev,const void *data,int len)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);
int i;
#ifdef CATCH_SIGPIPE
static struct sigaction saBlank;
struct sigaction saNew,saOld;
sigset_t oldMask,newMask;
#endif
#ifndef USE_THREADS
Environment ev2=RHBSOMD_EnvironmentInit;
#endif

	if ((somThis->fd == INVALID_SOCKET) || (!IS_CONNECTED(somThis)))
	{
		RHBSOM_Trace("bad send\n");

		*ev=SOMS_ENOTSOCK;

		return -1;
	}

	RHBSOM_Trace("send start")

#ifdef CATCH_SIGPIPE
#define JUST_SIGPIPE    1
	saNew=saBlank;
#if JUST_SIGPIPE
	sigfillset(&newMask);
	sigdelset(&newMask,SIGPIPE);
#else
	sigemptyset(&newMask);
	sigaddset(&newMask,SIGPIPE);
#endif
	sigemptyset(&saNew.sa_mask);
	saNew.sa_handler=catch_sigpipe;
#ifdef SA_RESTART
	saNew.sa_flags=SA_RESTART;
#endif
	sigaction(SIGPIPE,&saNew,&saOld);
#if JUST_SIGPIPE
	sigprocmask(SIG_SETMASK,&newMask,&oldMask);
#else
	sigprocmask(SIG_UNBLOCK,&newMask,&oldMask);
#endif
	#define CATCH_SIGPIPE_RESTORE	sigaction(SIGPIPE,&saOld,NULL); sigprocmask(SIG_SETMASK,&oldMask,NULL); 
#endif

#ifdef USE_THREADS
	i=send(somThis->fd,data,len,0);
#else
	RHBOPT_ASSERT(somThis->socketObject);
	{
		i=Sockets_somsSend(somThis->socketObject,&ev2,somThis->fd,(void *)data,len,0);
	}
#endif

	if (i != -1) 
	{
	#ifdef CATCH_SIGPIPE
		CATCH_SIGPIPE_RESTORE
	#endif
	}
	else
	{
		int err=
#ifdef USE_THREADS
		Sockets_GetError(
#	ifdef _WIN32
			WSAGetLastError()
#	else
			errno
#	endif
			);
#else
		Sockets__get_serrno(somThis->socketObject,&ev2);
#endif

#ifdef CATCH_SIGPIPE
		CATCH_SIGPIPE_RESTORE
#endif

		*ev=err;

		switch (err)
		{
		case SOMS_EWOULDBLOCK:
			break;
		default:
			somThis->state=RHBSOCK_STATE_BROKEN;
			break;
		}
	}

#ifndef USE_THREADS
	SOM_UninitEnvironment(&ev2);
#endif

	return i;
}

static RHBSocket * RHBLINK rhbbsd_Accept(RHBSocket *somSelf,
			RHBSocketError *ev,
			RHBSocketAddress *addr,
			RHBSocketLength *len,
			RHBSocketAddress *sockname,
			RHBSocketLength *socklen)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);
SOCKET a=INVALID_SOCKET;
#ifdef USE_THREADS
#else
Environment ev2=RHBSOMD_EnvironmentInit;
#endif

	if (somThis->fd == INVALID_SOCKET)
	{
		*ev=SOMS_ENOTSOCK;

		return NULL;
	}

#ifdef USE_THREADS
	a=accept(somThis->fd,addr,len);
#else
	RHBOPT_ASSERT(somThis->socketObject);
	a=Sockets_somsAccept(somThis->socketObject,&ev2,somThis->fd,addr,len);
#endif

	if (a!=INVALID_SOCKET)
	{
		if (IS_UNUSABLE_SOCKET(a))
		{
#ifdef USE_THREADS
			soclose(a);
#else
			Sockets_somsSoclose(somThis->socketObject,&ev2,a);
#endif
			a=INVALID_SOCKET;
		}
		else
		{
			close_on_exec(a);
		}
	}

	if (a != INVALID_SOCKET)
	{
		RHBSocketRef s=rhbbsd_create_from_fd(somThis->protocol,a,addr->sa_family);

		if (s)
		{
			RHBSocketGetData(s)->state=RHBSOCK_STATE_UP;

			if (sockname)
			{
				/* this gets my name */
#ifdef USE_THREADS
				getsockname(a,sockname,socklen);
#else
				RHBOPT_ASSERT(somThis->socketObject);
				Sockets_somsGetsockname(somThis->socketObject,&ev2,a,sockname,socklen);
#endif
			}

			return s;
		}

#ifdef USE_THREADS
		soclose(a);
#else
		Sockets_somsSoclose(somThis->socketObject,&ev2,a);
#endif
	}

	*ev=SOMS_ENOTSOCK;

	return NULL;
}

static boolean RHBLINK rhbbsd_IsConnected(RHBSocket *somSelf,RHBSocketError *ev)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);

	if (somThis->fd == INVALID_SOCKET) return 0;

	RHBOPT_unused(ev)

	return (boolean)(IS_CONNECTED(somThis) ? 1 : 0);
}

static void RHBLINK rhbbsd_Shutdown(RHBSocket *somSelf,RHBSocketError *ev,int how)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);

	RHBOPT_unused(ev)

	if (!IS_LISTENER(somThis))
	{
		RHBSocket_AddRef(somSelf);

		if (how /*==2*/)
		{
			SOCKET fd=INVALID_SOCKET;

			RHBORB_guard(0)

			fd=somThis->fd;

			if ((fd != INVALID_SOCKET) && IS_CONNECTED(somThis))
			{
				somThis->state=RHBSOCK_STATE_CLOSED;

#ifdef USE_THREADS
				shutdown(fd,1);
#else
				{
					Environment ev2=RHBSOMD_EnvironmentInit;
					Sockets_somsShutdown(somThis->socketObject,&ev2,fd,1);
					SOM_UninitEnvironment(&ev2);
				}
#endif
			}

			somThis->writes_wanted=0;

	#ifdef USE_THREADS
			somThis->task.selectFlags&=~(RHBSocket_select_write|RHBSocket_select_except);
			thread_blip(somThis->task.task_thread);
	#endif

			RHBORB_unguard(0)
		}

		RHBSocket_Release(somSelf);
	}
}

static void RHBLINK rhbbsd_StartListen(RHBSocket *somSelf,RHBSocketError *ev,
		RHBSocketAddress *addr,RHBSocketLength len,
		RHBSocketAddress *actual,RHBSocketLength *plen)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);
int i;
#ifndef USE_THREADS
Environment ev2=RHBSOMD_EnvironmentInit;
#endif

	VALIDATE_ADDRESS(addr,len)

	if (somThis->fd==INVALID_SOCKET)
	{
		*ev=SOMDERROR_SocketCreate;

		return;
	}

#ifdef USE_THREADS
	i=bind(somThis->fd,addr,len);
#else
	i=Sockets_somsBind(somThis->socketObject,&ev2,somThis->fd,addr,len);
#endif

	if (!i)
	{
	#ifdef USE_THREADS
		i=listen(somThis->fd,5);
	#else
		i=Sockets_somsListen(somThis->socketObject,&ev2,somThis->fd,5);
	#endif
	}

	if (i)
	{
#ifdef USE_THREADS
		*ev=Sockets_GetError(
#ifdef _WIN32
			WSAGetLastError()
#else
			errno
#endif
			);
#else
		*ev=Sockets__get_serrno(somThis->socketObject,&ev2);
#endif
	}
	else
	{
		somThis->local_addrlen=sizeof(somThis->local_addr);

#ifdef USE_THREADS
		getsockname(somThis->fd,&somThis->local_addr.abstract,&somThis->local_addrlen);
#else
		Sockets_somsGetsockname(somThis->socketObject,&ev2,somThis->fd,
			&somThis->local_addr.abstract,&somThis->local_addrlen);
#endif

		if (actual)
		{
			int len=*plen;
			if (len > somThis->local_addrlen)
			{
				len=somThis->local_addrlen;
			}
			if (len)
			{
				memcpy(actual,&somThis->local_addr.abstract,len);
			}
			*plen=len;
		}

		somThis->state=RHBSOCK_STATE_PASSIVE;
	}

#ifndef USE_THREADS
	SOM_UninitEnvironment(&ev2);
#endif
}

static boolean RHBLINK rhbbsd_StartConnect(RHBSocket *somSelf,RHBSocketError *ev,RHBSocketAddress *addr,RHBSocketLength len)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);
int i=-1;
#ifdef USE_THREADS
	RHBOPT_ASSERT(!somThis->task.selectFlags)
	RHBOPT_ASSERT(somThis->task.task_thread)
#else
	Environment ev2=RHBSOMD_EnvironmentInit;
	boolean bRetVal=0;
#endif

	VALIDATE_ADDRESS(addr,len)

	if (len > sizeof(somThis->remote_addr))
	{
		*ev=SOMS_ENAMETOOLONG;

		return 0;
	}

	if ((somThis->fd==INVALID_SOCKET)||(somThis->state!=RHBSOCK_STATE_IDLE))
	{
		*ev=SOMS_ENOTSOCK;

		return 0;
	}

	RHBORB_guard(0)

	memcpy(&somThis->remote_addr,addr,len);
	somThis->remote_addrlen=len;

	somThis->state=RHBSOCK_STATE_ACTIVE;
#if 1
	i=0;
#else
	switch (addr->sa_family)
	{
	case AF_INET:
		{
			struct sockaddr_in local_addr;

			memset(&local_addr,0,sizeof(addr));

#ifdef HAVE_SOCKADDR_SA_LEN
			local_addr.sin_len=sizeof(local_addr);
#endif
			local_addr.sin_family=AF_INET;
			local_addr.sin_addr.s_addr=htonl(INADDR_ANY);
			local_addr.sin_port=0;

			i=bind(somThis->fd,(struct sockaddr *)&local_addr,sizeof(local_addr));
		}
		break;
#ifdef HAVE_SOCKADDR_IN6
	case AF_INET6:
		{
			struct sockaddr_in6 local_addr;
			struct in6_addr any_addr=in6addr_any;

			memset(&local_addr,0,sizeof(local_addr));

#ifdef HAVE_SOCKADDR_SA_LEN
			local_addr.sin6_len=sizeof(local_addr);
#endif
			local_addr.sin6_family=AF_INET6;
			local_addr.sin6_addr=any_addr;
			local_addr.sin6_port=0;
			local_addr.sin6_flowinfo=0;

			i=bind(somThis->fd,(struct sockaddr *)&local_addr,sizeof(local_addr));
		}
		break;
#endif
	}
#endif
	if (i)
	{
		somThis->state=RHBSOCK_STATE_BROKEN;

#ifdef USE_THREADS
		*ev=Sockets_GetError(
#	ifdef _WIN32
			WSAGetLastError()
#	else
			errno
#	endif
			);
#else
		*ev=Sockets__get_serrno(somThis->socketObject,&ev2);
#endif

		RHBORB_unguard(0)

		return 0;
	}

	RHBOPT_ASSERT(somThis->fd != INVALID_SOCKET)

	switch (somThis->remote_addr.abstract.sa_family)
	{
	case AF_INET:
		debug_somPrintf(("connect(%s:%d)\n",
			inet_ntoa(somThis->remote_addr.in.sin_addr),
			ntohs(somThis->remote_addr.in.sin_port)));
		break;
#ifdef HAVE_SOCKADDR_IN6
	case AF_INET6:
		{
#ifdef _DEBUG
/*			char buf[256]={0};*/
#endif
			#if 0
			if (IN6_IS_ADDR_V4MAPPED(&somThis->remote_addr.in6.sin6_addr))
			{
				unsigned char *addrPtr=(void *)&somThis->remote_addr.in6.sin6_addr;
				struct in_addr addr4;
				/* swizzle IPV4 loopback to IPV6 loopback */

				memcpy(&addr4,addrPtr+12,sizeof(addr));

				if (addr4.s_addr==htonl(INADDR_LOOPBACK))
				{
					struct in6_addr a6=IN6ADDR_LOOPBACK_INIT;
					somThis->remote_addr.in6.sin6_addr=a6;
				}
			}
			#endif

/*			debug_somPrintf(("connect6(%s:%d)\n",
				inet_ntop(somThis->remote_addr.in6.sin6_family,
					&somThis->remote_addr.in6.sin6_addr,
					buf,sizeof(buf)),
				ntohs(somThis->remote_addr.in6.sin6_port)));*/

		}
		break;
#endif
	}

#ifdef USE_THREADS
	if (somThis->task.task_thread)
	{
	/* do the connect in the selecting thread */
		somThis->task.selectFlags=RHBSocket_select_connect;

		*ev=SOMS_EINPROGRESS;

		thread_blip(somThis->task.task_thread);
	}
	else
	{
		*ev=SOMS_ENOBUFS;
	}

	RHBORB_unguard(0)

	return 0;
#else
	i=Sockets_somsConnect(somThis->socketObject,&ev2,
						somThis->fd,
						&(somThis->remote_addr.abstract),
						somThis->remote_addrlen);

	if (i != -1)
	{
		bRetVal=1;

		somThis->state=RHBSOCK_STATE_UP;
	}
	else
	{
		long err=Sockets__get_serrno(somThis->socketObject,&ev2);

		*ev=err;

		switch (err)
		{
		case SOMS_EWOULDBLOCK:
		case SOMS_EINPROGRESS:
			break;
		default:
			somThis->state=RHBSOCK_STATE_BROKEN;
			break;
		}
	}

	RHBORB_unguard(0)

	return bRetVal;
#endif
}

#ifdef USE_THREADS
static void rhbbsd_removed_locked(struct thread_task *task)
{
#ifdef USE_PTHREADS
	pthread_cond_t *kick=
#else
	HANDLE kick=
#endif
	task->data->sock->impl->kick_on_close;

	/* now require this to be done by whoever requested the remove
	somd_atomic_inc(&(task->data->lUsage));
	*/
	
	RHBOPT_ASSERT(RHBSocket_select_remove & task->selectFlags);
	RHBOPT_ASSERT(task->data->lUsage);

	task->selectFlags&=~(RHBSocket_select_remove);

	task->data->sock->impl->kick_on_close=NULL;

	RHBSOM_Trace("rhbbsd_removed_locked");

	if (kick)
	{
		RHBSOM_Trace("rhbbsd_removed_locked - signalling");
#ifdef USE_PTHREADS
		pthread_cond_signal(kick);
#else
		if (!SetEvent(kick)) 
		{
#ifdef _M_IX86
			__asm int 3;
#endif
		}
#endif
	}

	RHBSocket_Release(task->data->sock);
}
#endif

static void rhbbsd_set_options(RHBSocketData *somThis)
{
int i;
#if defined(_WIN32)||defined(_WIN16)
BOOL b=1;
#else
int b=1;
#endif
#ifdef USE_THREADS
#else
Environment ev2=RHBSOMD_EnvironmentInit;
#endif
	/* various docs say don't need to set TCP_NODELAY
		but have found it runs like a dog without it,
		difference is 

			without, 25 request/responses a second
			with	1000 request/responses a second

		between processes */
		
#ifdef USE_THREADS
	i=setsockopt(somThis->fd,IPPROTO_TCP,
		TCP_NODELAY,(char *)&b,sizeof(b));
#else
	i=Sockets_somsSetsockopt(
		somThis->socketObject,&ev2,
		somThis->fd,IPPROTO_TCP,
		TCP_NODELAY,(char *)&b,sizeof(b));
#endif

	if (i)
	{
		RHBSOM_Trace("Did not set TCP_NODELAY")
	}

	b=1;

#ifdef USE_THREADS
	i=setsockopt(somThis->fd,SOL_SOCKET,
		SO_KEEPALIVE,(char *)&b,sizeof(b));
#else
	i=Sockets_somsSetsockopt(
		somThis->socketObject,&ev2,
		somThis->fd,SOL_SOCKET,
		SO_KEEPALIVE,(char *)&b,sizeof(b));
#endif

	if (i)
	{
		RHBSOM_Trace("Did not set SO_KEEPALIVE")
	}

	/* alternative is MSG_NOSIGNAL in send() */

#ifdef SO_NOSIGPIPE
	b=1;

#ifdef USE_THREADS
	i=setsockopt(somThis->fd,SOL_SOCKET,
		SO_NOSIGPIPE,(char *)&b,sizeof(b));
#else
	i=Sockets_somsSetsockopt(
		somThis->socketObject,&ev2,
		somThis->fd,SOL_SOCKET,
		SO_NOSIGPIPE,(char *)&b,sizeof(b));
#endif

	if (i)
	{
		RHBSOM_Trace("Did not set SO_NOSIGPIPE")
	}
#endif
}

static RHBSocketRef rhbbsd_create_from_fd(RHBProtocolRef protocol,SOCKET fd,int family)
{
	struct 
	{
		RHBSocket sock;
		RHBSocketData data;
	} *a=SOMMalloc(sizeof(*a));

	RHBOPT_ASSERT(fd!=INVALID_SOCKET)

	if (a)
	{
		memset(a,0,sizeof(*a));
		a->sock.vtbl=&rhbbsd_JumpTable;
		a->sock.impl=&a->data;
		a->data.sink=0;
		a->data.protocol=protocol;
		a->data.lUsage=1;
		a->data.fd=fd;
		a->data.sock=&a->sock;
		a->data.state=RHBSOCK_STATE_IDLE;

		protocol->vtbl->AddRef(protocol);

		RHBORB_guard(orb);

		a->data.next=protocol->impl->socket_impl_list;
		protocol->impl->socket_impl_list=&a->data;

#ifdef USE_THREADS
		a->data.task.removed_locked=rhbbsd_removed_locked;
		a->data.task.data=&a->data;
		a->data.task.fd=fd;

		if (!rhbbsdp_get_thread(protocol,&a->data.task,family))
		{
			soclose(fd);

			fd=INVALID_SOCKET;
		}
#else
		a->data.socketObject=protocol->impl->socketObject;
#endif

		if (fd==INVALID_SOCKET)
		{
			if (protocol->impl->socket_impl_list==&a->data)
			{
				protocol->impl->socket_impl_list=a->data.next;
			}
			else
			{
				struct RHBSocketData *p=protocol->impl->socket_impl_list;

				while (p->next != &a->data)
				{
					p=p->next;
				}

				p->next=a->data.next;
			}
		}
		
		RHBORB_unguard(orb);

		if (fd != INVALID_SOCKET)
		{
			rhbbsd_set_options(&a->data);

			return &a->sock;
		}

		SOMFree(a);

		protocol->vtbl->Release(protocol);
	}

	return NULL;
}

static RHBSocketRef rhbbsd_create(RHBProtocolRef protocol,unsigned short family)
{
	RHBSocketRef ref=NULL;
#ifdef USE_THREADS
	SOCKET fd=fd=socket(family,SOCK_STREAM,0);
#else
	Environment ev2=RHBSOMD_EnvironmentInit;
	Sockets SOMSTAR socketObject=protocol->impl->socketObject;
	SOCKET fd=Sockets_somsSocket(socketObject,&ev2,family,SOCK_STREAM,0);
#endif
	
	if (fd!=INVALID_SOCKET)
	{
		if (IS_UNUSABLE_SOCKET(fd))
		{
#ifdef USE_THREADS
			soclose(fd);
#else
			Sockets_somsSoclose(socketObject,&ev2,fd);
#endif
			fd=INVALID_SOCKET;
		}
		else
		{
			close_on_exec(fd);
		}
	}

	if (fd!=INVALID_SOCKET)
	{
		ref=rhbbsd_create_from_fd(protocol,fd,family);
	}

	return ref;
}

static void RHBLINK rhbbsd_Close(RHBSocket *somSelf)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);
SOCKET fd=INVALID_SOCKET;
RHBSocketSink *sink;
#ifdef USE_THREADS
#else
Environment ev2=RHBSOMD_EnvironmentInit;
#endif
/*	somPrintf("rhbbsd_Close(%d)\n",somThis->fd);*/

	RHBSOM_Trace("rhbbsd_Close - begin");

	RHBORB_guard(0)

	sink=somThis->sink;
	somThis->sink=NULL;

	fd=somThis->fd;

#ifndef USE_THREADS
	somThis->fd=INVALID_SOCKET;
#endif

	if (!IS_LISTENER(somThis))
	{
		RHBSOM_Trace("rhbbsd_Close - is normal socket");

		if (fd != INVALID_SOCKET)
		{
			/* this should stop the receiver... */
			if (IS_CONNECTED(somThis))
			{
#ifdef USE_THREADS
				shutdown(fd,1);
#else
				Sockets_somsShutdown(somThis->socketObject,&ev2,fd,1);
#endif

				somThis->state=RHBSOCK_STATE_BROKEN;
			}
#ifdef USE_THREADS
			if (thread_task_detach(&somThis->task))
			{
				/* it's detached, we clean up the socket */
				somThis->fd=INVALID_SOCKET;
			}
			else
			{
				/* need to tell the selector to drop it when it's ready */

				RHBOPT_ASSERT(!(somThis->task.selectFlags&RHBSocket_select_remove));

				somd_atomic_inc(&somThis->lUsage);

				somThis->task.selectFlags|=RHBSocket_select_remove;

				thread_blip(somThis->task.task_thread);

				/* if selector is running, it'll clean up the socket */

				fd=INVALID_SOCKET;
			}
#endif
		}
	}
	else
	{
		RHBSOM_Trace("rhbbsd_Close - is listener");

#ifdef USE_THREADS
		if ((somThis->task.selectFlags & RHBSocket_select_read) && !somThis->kick_on_close)
		{
#ifdef USE_PTHREADS
			pthread_cond_t pEvent;
			pthread_cond_init(&pEvent,RHBOPT_pthread_condattr_default);
			somThis->kick_on_close=&pEvent;
#else
			HANDLE hEvent=CreateEvent(NULL,0,0,NULL);
			somThis->kick_on_close=hEvent;
#endif

			RHBOPT_ASSERT(!(somThis->task.selectFlags&RHBSocket_select_remove));

			somThis->task.selectFlags=RHBSocket_select_remove;

			RHBSOM_Trace("rhbbsd_Close - while kick_on_close");

			somd_atomic_inc(&somThis->lUsage);

			thread_blip(somThis->task.task_thread);

			while (somThis->kick_on_close)
			{
/*				unsigned long ul=~0UL; */

				RHBSOM_Trace("rhbbsd_Close - blipped");

/*				somPrintf("kick_on_close %p,ul=%ld\n",somThis->kick_on_close,ul);*/

#if defined(USE_PTHREADS)
				somd_wait(&pEvent,__FILE__,__LINE__);
#else
				somd_wait(hEvent,__FILE__,__LINE__);
#endif
			}

#ifdef USE_PTHREADS
			pthread_cond_destroy(&pEvent);
#else
			if (!CloseHandle(hEvent)) 
			{
#ifdef _M_IX86
				__asm int 3;
#endif
			}
#endif
		}
#endif

		RHBSOM_Trace("rhbbsd_Close - stopped listener");
	}

	RHBSOM_Trace("rhbbsd_Close - closing socket");

	if (fd != INVALID_SOCKET) 
	{
		somThis->fd=INVALID_SOCKET;

#ifdef USE_THREADS
		soclose(fd);
#else
		Sockets_somsSoclose(somThis->socketObject,&ev2,fd);
#endif
	}

	RHBORB_unguard(0)

#ifdef USE_SELECT
	if (somThis->event_token)
	{
		RHBORB_remove_socket_event(somThis->protocol->impl->orb,somThis->event_token);

		somThis->event_token=0;
	}
#endif

	RHBSOM_Trace("rhbbsd_Close - releasing sink");

	if (sink) RHBSocketSink_Release(sink);

	RHBSOM_Trace("rhbbsd_Close - done");
}

static void RHBLINK rhbbsd_EnableWrites(RHBSocket * somSelf,boolean how)
{
RHBSocketData *somThis=RHBSocketGetData(somSelf);

	if (IS_CONNECTED(somThis))
	{
#ifdef USE_SELECT
		if (how != somThis->writes_wanted)
		{
			if (how)
			{
				RHBORB_set_socket_mask(
					somThis->protocol->impl->orb,
					somThis->event_token,
					EMInputReadMask|EMInputWriteMask);
			}
			else
			{
				RHBORB_set_socket_mask(
					somThis->protocol->impl->orb,
					somThis->event_token,
					EMInputReadMask);
			}
			somThis->writes_wanted=how;
		}
#else
/*		if (how != somThis->writes_wanted)*/
		{
			RHBORB_guard(0)

/*			if (how != somThis->writes_wanted)*/
			{
				somThis->writes_wanted=how;
			/* pthread_signal writer thread */
				if (how)
				{
					somThis->task.selectFlags=RHBSocket_select_read|RHBSocket_select_write;
				}
				else
				{
					somThis->task.selectFlags=RHBSocket_select_read;
				}

				if (somThis->task.task_thread)
				{
					thread_blip(somThis->task.task_thread);
				}
			}

			RHBORB_unguard(0)
		}
#endif
	}
}

#ifdef USE_THREADS
static void rhbbsd_do_notify(
			struct rhbbsd_thread_selector *selector,
			struct thread_task *task,
			int how)
{
	RHBSocketSink *sink=task->data->sink;
	struct RHBSocketData *sock=task->data;

	if (sink)
	{
		short msg=0;
		long err=0;

		if (how & RHBSocket_select_connect)
		{
			RHBOPT_ASSERT(IS_CONNECTING(sock))

			RHBOPT_ASSERT(task->selectFlags==RHBSocket_select_connect)

			if (IS_CONNECTING(sock))
			{
				int i;

				task->selectFlags&=~(RHBSocket_select_read|RHBSocket_select_write|RHBSocket_select_except|RHBSocket_select_connect);

				i=connect(sock->fd,&sock->remote_addr.abstract,sock->remote_addrlen);

				if (i==-1)
				{
				#ifdef _WIN32
					int en=WSAGetLastError();
					if ((en==WSAEWOULDBLOCK)||(en==WSAEINPROGRESS))
				#else
					int en=errno;
					if ((en==EWOULDBLOCK)||(en==EINPROGRESS))
				#endif
					{
						task->selectFlags|=RHBSocket_select_write|RHBSocket_select_except;
					}
					else
					{
						msg=RHBSocket_notify_connect;
						err=SOMDERROR_CannotConnect;
					}
				}
				else
				{
					msg=RHBSocket_notify_connect;
					sock->state=RHBSOCK_STATE_UP;
					task->selectFlags=RHBSocket_select_read|RHBSocket_select_write;
				}
			}
		}
		else
		{
			if (how & (RHBSocket_select_read|RHBSocket_select_write|RHBSocket_select_except))
			{
				if (how & RHBSocket_select_except)
				{
					if (IS_CONNECTING(sock))
					{
						sock->state=RHBSOCK_STATE_BROKEN;

						msg=RHBSocket_notify_connect;

						err=SOMDERROR_CannotConnect;

						/* don't do any more selecting on this */

						task->selectFlags&=~(RHBSocket_select_read|RHBSocket_select_write|RHBSocket_select_except);
					}
					else
					{
						msg=RHBSocket_notify_except;

						err=SOMDERROR_SocketSelect;
					}
				}
				else
				{
					/* it is a priority to do a write rather
						than a read because we have data
						we want to get rid of before we
						start accepting more data in */

					if (how & RHBSocket_select_write)
					{
						if (IS_CONNECTING(sock))
						{
							sock->state=RHBSOCK_STATE_UP;

							task->selectFlags=RHBSocket_select_read|RHBSocket_select_write;

							msg=RHBSocket_notify_connect;
						}
						else
						{
							msg=RHBSocket_notify_write;
						}
					}
					else
					{
						if (how & RHBSocket_select_read)
						{
							if (IS_LISTENER(sock))
							{
								msg=RHBSocket_notify_accept;
							}
							else
							{
								msg=RHBSocket_notify_read;
							}
						}
					}
				}

			}
		}

		if (msg)
		{
			RHBSocketSink *sink=task->data->sink;
			if (sink)
			{
				RHBSocketSink_AddRef(sink);
			
				RHBORB_unguard(0)
				selector->locked=0;

				RHBSocketSink_Notify(sink,sock->sock,msg,err);
	
				RHBSocketSink_Release(sink);

				selector->locked=1;
				RHBORB_guard(0)
			}
		}
	}
}
#else
static void rhbbsd_Callback(Environment *ev,SOCKET sock,int mask,void *pv)
{
	struct RHBSocket *refCon=pv;
	RHBSocketData *somThis=refCon->impl;
	RHBSocketSink *sink=somThis->sink;

	if (sink) RHBSocketSink_AddRef(sink);

	if (mask & EMInputExceptMask)
	{
		if (IS_CONNECTING(somThis))
		{
			somThis->state=RHBSOCK_STATE_BROKEN;

			if (sink)
			{
				RHBSocketSink_Notify(
					sink,
					somThis->sock,
					RHBSocket_notify_connect,
					SOMDERROR_CannotConnect);
			}
		}
		else
		{
			if (sink)
			{
				RHBSocketSink_Notify(
					sink,
					somThis->sock,
					RHBSocket_notify_except,
					SOMDERROR_SocketSelect);
			}
		}
	}

	if (mask & EMInputWriteMask)
	{
		if (IS_CONNECTED(somThis))
		{
			if (sink)
			{
				RHBSocketSink_Notify(
					sink,
					somThis->sock,
					RHBSocket_notify_write,
					0);
			}
		}
		else
		{
			somThis->state=RHBSOCK_STATE_UP;

			if (sink)
			{
				RHBSocketSink_Notify(
				sink,
				somThis->sock,
				RHBSocket_notify_connect,
				0);
			}
		}
	}

	if (mask & EMInputReadMask)
	{
		if (sink)
		{
			if (IS_CONNECTED(somThis))
			{
				RHBSocketSink_Notify(
					sink,
					somThis->sock,
					RHBSocket_notify_read,
					0);
			}
			else
			{
				RHBSocketSink_Notify(
					sink,
					somThis->sock,
					RHBSocket_notify_accept,
					0);
			}
		}
	}

	if (sink) RHBSocketSink_Release(sink);
}
#endif

static void RHBLINK rhbbsd_IsNonBlocking(RHBSocket * somSelf,RHBSocketSink *cb)
{
	RHBSocketData *somThis=RHBSocketGetData(somSelf);

/*	somPrintf("IsNonBlocking(%p,%p)\n",somSelf,cb);*/

	RHBORB_guard(0)

	if (cb != somThis->sink)
	{
		RHBSocketSink *old=somThis->sink;
		if (cb) RHBSocketSink_AddRef(cb);
		somThis->sink=cb;
		if (old) RHBSocketSink_Release(old);
	}

	if (cb)
	{
		somThis->writes_wanted=1;

		set_non_blocking(
#if !defined(USE_THREADS)
			somThis->socketObject,
#endif
			somThis->fd);

#ifdef USE_SELECT
		somThis->event_token=
			RHBORB_add_socket_event(
				somThis->protocol->impl->orb,
				somThis->fd,
		/*		RHBImplementationDef_Callback*/ rhbbsd_Callback,
				somSelf);
		RHBORB_set_socket_mask(
			somThis->protocol->impl->orb,
			somThis->event_token,
			IS_LISTENER(somThis) ? EMInputReadMask : (EMInputReadMask|EMInputExceptMask|EMInputWriteMask));
#else
		somThis->task.notify=rhbbsd_do_notify;

		if (somThis->sink)
		{
			if (IS_LISTENER(somThis))
			{
				somThis->task.selectFlags|=RHBSocket_select_read;
			}
			else
			{
				if (IS_CONNECTED(somThis))
				{
					somThis->task.selectFlags|=RHBSocket_select_read|RHBSocket_select_write;
				}
				else
				{
					if (IS_CONNECTING(somThis))
					{
						somThis->task.selectFlags|=RHBSocket_select_write|RHBSocket_select_except;
					}
				}
			}

			if (somThis->task.selectFlags)
			{
				thread_blip(somThis->task.task_thread);
			}
		}
#endif
	}

	RHBORB_unguard(0)
}

typedef struct RHBProtocolData RHBProtocolData;

#define RHBProtocolGetData(x)   x->impl

RHBProtocolRef SOMLINK RHBORB_create_protocol(RHBORB *orb)
{
	struct 
	{
		RHBProtocol protocol;
		struct RHBProtocolData data;
	} *a;

#if defined(_WIN32) && defined(USE_THREADS)
	WSADATA wsd;
	WSAStartup(0x101,&wsd);
#endif

	a=SOMMalloc(sizeof(*a));
	memset(a,0,sizeof(*a));
	a->data.lUsage=1;
	a->data.closing=0;

	a->data.orb=orb;

	a->data.socket_impl_list=0;

	a->protocol.impl=&a->data;
	a->protocol.vtbl=&rhbbsdp_JumpTable;

#ifdef USE_THREADS
#else
	a->data.socketObject=orb->events.socketObject;
#endif

	return &a->protocol;
}

static int RHBLINK rhbbsdp_QueryInterface(RHBProtocolRef somSelf,void *iid,void **ppv)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(iid)
	RHBOPT_unused(ppv)
	return -1;
}

static int RHBLINK rhbbsdp_AddRef(RHBProtocolRef somSelf)
{
RHBProtocolData *somThis=RHBProtocolGetData(somSelf);

	somd_atomic_inc(&somThis->lUsage);

	return 0;
}

static void RHBLINK rhbbsdp_Close(RHBProtocolRef somSelf)
{
RHBProtocolData *somThis=RHBProtocolGetData(somSelf);

	RHBORB_guard(0)

	if (!somThis->closing)
	{
#ifdef USE_THREADS
		somThis->closing=1;

		while (somThis->threads)
		{
			struct thread_select *t=somThis->threads;

			somThis->threads=t->next;
			t->protocol=NULL;
			thread_blip(t);
		}

		somThis->closing=0;
#endif
		RHBOPT_ASSERT(!somThis->resolvers)

		while (somThis->socket_impl_list) 
		{
			struct RHBSocketData *sock=somThis->socket_impl_list;
			somThis->socket_impl_list=sock->next;
			
			RHBOPT_ASSERT(!somThis)	/* just a debug warning */

			if (sock->protocol==somSelf)
			{
				sock->protocol=NULL;

#ifdef USE_THREADS
				/* the act of removal will drop the count */
				sock->task.selectFlags=RHBSocket_select_remove;
				thread_blip(sock->task.task_thread);
#else
				if (!somd_atomic_dec(&somThis->lUsage))
				{
					RHBOPT_ASSERT(!somThis) /* just a debug warning */
				}
#endif
			}
		}
	}

	RHBORB_unguard(0)
}

static int RHBLINK rhbbsdp_Release(RHBProtocolRef somSelf)
{
RHBProtocolData *somThis=RHBProtocolGetData(somSelf);

	if (somd_atomic_dec(&somThis->lUsage)) return 0;

	RHBProtocol_Close(somSelf);

	if (somThis->lUsage) return 0;

	SOMFree(somSelf);

#if defined(_WIN32) && defined(USE_THREADS)
	WSACleanup();
#endif

	return 0;
}

static RHBSocketRef RHBLINK rhbbsdp_CreateStream(RHBProtocolRef somSelf,unsigned short family)
{
	return rhbbsd_create(somSelf,family);
}

typedef struct RHBProtocol_hostent
{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		struct hostent he;
		#ifdef HAVE_HOSTENT_DATA
			struct hostent_data hd;
		#else
		#endif
		char buf[1024];
		int h_error;
	#else
		char zilch_r[1];
	#endif
#else
	char zilch[1];
#endif
} RHBProtocol_hostent;

#if !defined(HAVE_GETIPNODEBYNAME) && !defined(HAVE_GETADDRINFO) && defined(USE_THREADS) && defined(USE_PTHREADS)
static struct hostent * SOMLINK rhbbsdp_GetHostByName_r(
		RHBProtocolRef somSelf,char *name,struct RHBProtocol_hostent *he)
{
	struct hostent *hp=NULL;
	memset(he,0,sizeof(he[0]));
	#ifdef HAVE_HOSTENT_DATA
	hp=&(he->he);
	if (gethostbyname_r(name,
		hp,&he->hd))
	{
		hp=NULL;
	}
	#else /* HAVE_HOSTENT_DATA */
		#ifdef HAVE_GETHOSTBYNAME_R_LINUX
		if (gethostbyname_r(name,
		&he->he,he->buf,sizeof(he->buf),&hp,&he->h_error))
		{
			hp=NULL;
		}
		#else /* HAVE_GETHOSTBYNAME_R_LINUX */
			#ifdef HAVE_GETHOSTBYNAME_R_SOLARIS
				hp=gethostbyname_r(name,
					&he->he,
					he->buf,sizeof(he->buf),
					&he->h_error);
			#else /* HAVE_GETHOSTBYNAME_R_SOLARIS */
				hp=gethostbyname(name);
			#endif /* !HAVE_GETHOSTBYNAME_R_SOLARIS */
		#endif /* !HAVE_GETHOSTBYNAME_R_LINUX */
	#endif /* HAVE_HOSTENT_DATA */
	return hp;
}
#else
	#define rhbbsdp_GetHostByName_r(p,n,e) gethostbyname(n)
#endif

static RHBResolver * RHBLINK rhbbsdp_CreateResolver(RHBProtocolRef somSelf,const char *name,unsigned short port)
{
	size_t len=strlen(name);
	RHBProtocolData *somThis=RHBProtocolGetData(somSelf);
	struct 
	{
		RHBResolver resolver;
		RHBResolverData data;
		char name[1];
	} *a=SOMMalloc(sizeof(*a)+len);

	memset(a,0,sizeof(*a));

#ifdef USE_RESOLVER_CHILD
	a->data.fd=INVALID_SOCKET;
	a->data.packet_size=sizeof(a->data.length_bytes);
	a->data.length_bytes_len=0;
#ifdef _WIN32
	a->data.hThread=NULL;
#else
	a->data.pid=0;
#endif	/* _WIN32 */
#endif /* USE_RESOLVER_CHILD */

	RHBProtocol_AddRef(somSelf);

	a->data.protocol=somSelf;
	a->data.lUsage=1;
	a->data.name=a->name;
	a->data.port=port;

	memcpy(a->data.name,name,len+1);

	a->resolver.vtbl=&rhbbsdnr_JumpTable;
	a->resolver.impl=&a->data;
	a->data.cb=0;
	a->data.impl=0;
	a->data.resolver=&a->resolver;

	RHBORB_guard(0)

	a->data.next=somThis->resolvers;
	somThis->resolvers=&a->data;

	RHBORB_unguard(0)

	return &a->resolver;
}

#ifdef USE_THREADS
	static void SOMLINK rhbbsdr_thread_failed(void *);
#endif

#ifdef USE_RESOLVER_CHILD
	static void write_bytes(struct dnr_async *dnr,void *pv,size_t len)
	{
		if ((dnr->data._length+len) > dnr->data._maximum)
		{
			octet *_buffer=dnr->data._buffer;
			dnr->data._maximum+=(1024+len);
			dnr->data._buffer=malloc(dnr->data._maximum);
			if (dnr->data._length)
			{
				memcpy(dnr->data._buffer,_buffer,dnr->data._length);
			}
			if (_buffer) free(_buffer);
		}
		memcpy(dnr->data._buffer+dnr->data._length,pv,len);
		dnr->data._length+=len;
	}

	static int write_flush(struct dnr_async *dnr)
	{
		int rc=0;
		/* send four bytes of length first... */

		if (sizeof(dnr->data._length)==4)
		{
			int i=send(dnr->fd,(void *)&dnr->data._length,sizeof(dnr->data._length),0);
			if (i != 4) return -1;
		}
		else
		{
			int len=dnr->data._length;
			int i=send(dnr->fd,(void *)&len,sizeof(len),0);
			if (i != 4) return -1;
		}

		if (dnr->data._buffer)
		{
			char *p=(void *)dnr->data._buffer;
			int len=dnr->data._length;
			while (len)
			{
				int k=send(dnr->fd,p,len,0);
				if (k > 0)
				{
					p+=k;
					len-=k;
				}
				else
				{
					rc=-1;
					break;
				}
			}
			free(dnr->data._buffer);
			dnr->data._buffer=NULL;
			dnr->data._length=0;
			dnr->data._maximum=0;
		}

		return rc;
	}

	static void write_long(struct dnr_async *dnr,long l)
	{
		if (sizeof(l)==4)
		{
			write_bytes(dnr,(char *)&l,sizeof(l));
		}
		else
		{
			int i=(int)l;
			write_bytes(dnr,(char *)&i,sizeof(i));
		}
	}

	static void write_short(struct dnr_async *dnr,short s)
	{
		if (sizeof(s)==2)
		{
			write_bytes(dnr,(char *)&s,sizeof(s));
		}
		else
		{
			int i=s;
			write_bytes(dnr,(char *)&i,sizeof(i));
		}
	}

	static void my_ignore(int i)
	{
		signal(i,my_ignore);
	}

	static void my_quit(int i)
	{
		_exit(1);
	}

	static int dnr_async(struct dnr_async *dnr)
	{
		int rc=1;
#ifdef HAVE_GETADDRINFO
		struct addrinfo *gai=NULL;
#else
		struct hostent *hp;
#endif

#ifdef _WIN32
#else
		/* want parent able to kill me */
		sigset_t sigs;

		sigemptyset(&sigs);
		sigaddset(&sigs,SIGPIPE);
		sigaddset(&sigs,SIGTERM);
		sigaddset(&sigs,SIGINT);
		sigaddset(&sigs,SIGABRT);

		signal(SIGPIPE,my_quit);
		signal(SIGTERM,my_quit);
		signal(SIGINT,my_quit);
		signal(SIGABRT,my_quit);

		sigprocmask(SIG_UNBLOCK,&sigs,NULL);
#endif

					FORK_STATUS
#ifdef HAVE_GETADDRINFO
		if (!getaddrinfo(dnr->name,NULL,NULL,&gai))
		{
			int addrCount=0;
			struct addrinfo *p=gai;

			while (p)
			{
				if (do_include(p))
				{
					addrCount++;
				}

				p=p->ai_next;
			}

			write_long(dnr,1);

			write_long(dnr,addrCount);

			p=gai;

			while ((addrCount--) && p)
			{
				if (do_include(p))
				{
					if (p->ai_canonname)
					{
						int i=strlen(p->ai_canonname)+1;
						int pad=0;
						write_long(dnr,i);
						write_bytes(dnr,p->ai_canonname,i);
						while (i & 3)
						{
							i++;
							pad++;
						}
						if (pad)
						{
							static char pd[]={0,0,0};
							write_bytes(dnr,pd,pad);
						}
					}
					else
					{
						write_long(dnr,0);
					}

					write_short(dnr,(short)p->ai_family);
					write_short(dnr,0);
					write_long(dnr,p->ai_addrlen);
					switch (p->ai_family)
					{
					case AF_INET:
						{
							struct sockaddr_in *addr=(void *)p->ai_addr;
							addr->sin_port=htons(dnr->port);
						}
						break;
#ifdef HAVE_SOCKADDR_IN6
					case AF_INET6:
						{
							struct sockaddr_in6 *addr=(void *)p->ai_addr;
							addr->sin6_port=htons(dnr->port);
						}
						break;
#endif
					}
					write_bytes(dnr,(char *)p->ai_addr,p->ai_addrlen);
					if (p->ai_addrlen & 3)
					{
						int i=p->ai_addrlen;
						int pad=0;
						while (i & 3)
						{
							i++;
							pad++;
						}
						if (pad)
						{
							static char pd[]={0,0,0};
							write_bytes(dnr,pd,pad);
						}
					}
				}

				p=p->ai_next;
			}

			freeaddrinfo(gai);

			rc=0;
		}
#else
					FORK_STATUS
		hp=gethostbyname(dnr->name);

		if (hp)
		{
			int addrCount=0;
			char **h=hp->h_addr_list;

					FORK_STATUS
			while (hp->h_addr_list[addrCount])
			{
				addrCount++;
			}

			write_long(dnr,1); /* magically becomes byte order
									  for a CDR encapsulation */
			write_long(dnr,addrCount);

			while ((addrCount--) && hp)
			{
				char *p=*h++;

				write_long(dnr,0);					/* name */
				write_short(dnr,hp->h_addrtype);	/* family */
				write_short(dnr,0);					/* padding */
				
				switch (hp->h_addrtype)
				{
				case AF_INET:
					{
						struct sockaddr_in addr;
						memset(&addr,0,sizeof(addr));
						addr.sin_family=hp->h_addrtype;
						addr.sin_port=htons(dnr->port);
						memcpy(&addr.sin_addr,p,sizeof(addr.sin_addr));
						write_long(dnr,sizeof(addr));
						write_bytes(dnr,(char *)&addr,sizeof(addr));
					}
					break;
#ifdef HAVE_SOCKADDR_IN6
				case AF_INET6:
					{
						struct sockaddr_in6 addr;
						memset(&addr,0,sizeof(addr));
						addr.sin6_family=hp->h_addrtype;
						addr.sin6_port=htons(dnr->port);
						memcpy(&addr.sin6_addr,p,sizeof(addr.sin6_addr));
						write_long(dnr,sizeof(addr));
						write_bytes(dnr,(char *)&addr,sizeof(addr));
					}
					break;
#endif
				default:
					hp=NULL;
					break;
				}
			}

			rc=0;
		}
#endif
		FORK_STATUS

		if (write_flush(dnr))
		{
			FORK_STATUS
		}
		else
		{
			char buf[1];
			int len;

			FORK_STATUS
			len=recv(dnr->fd,buf,sizeof(buf),0);
			FORK_STATUS
		}

		FORK_STATUS
		soclose(dnr->fd);
		FORK_STATUS

		/* don't want to send any more */

		return rc;
	}
 
#ifdef _WIN32
	static DWORD __stdcall dnr_async_r(void *pv)
	{
		int rc=dnr_async(pv);

		free(pv);

		return rc;
	}
#endif
#endif

static void RHBLINK rhbbsdnr_Start(RHBResolverRef somSelf,
								   RHBGIOPRequestStream *impl,
								   RHBResolver_Callback cb)
{
RHBResolverData *somThis=somSelf->impl;

	RHBImplementationDef_AddRef(impl);

	RHBORB_guard(0)

	RHBSOM_Trace("resolving name")
	RHBSOM_Trace(somThis->name)

	somThis->impl=impl;
	somThis->cb=cb;

	if (rhbbsd_IsJustIPV4Address(somThis->name))
	{
#ifdef USE_THREADS
		uinteger4 addr4=inet_addr(somThis->name);
#else
		Environment ev2=RHBSOMD_EnvironmentInit;
		uinteger4 addr4=Sockets_somsInet_addr(
			somThis->protocol->impl->socketObject,
			&ev2,
			somThis->name);
#endif

		if (addr4 != /* INADDR_NONE */ (uinteger4)-1L)
		{
			struct sockaddr_in addr_in;
			_IDL_SEQUENCE_SOMD_NetworkAddress addrSeq={1,1,NULL};
			SOMD_NetworkAddress addr={NULL,AF_INET,
							{sizeof(addr_in),
							sizeof(addr_in),NULL}};
			addr.value._buffer=(octet *)&addr_in;
			addrSeq._buffer=&addr;
			memset(&addr_in,0,sizeof(addr_in));
			addr_in.sin_family=AF_INET;
#ifdef HAVE_SOCKADDR_SA_LEN
			addr_in.sin_len=sizeof(addr_in);
#endif
			addr_in.sin_port=htons(somThis->port);
			addr_in.sin_addr.s_addr=addr4;

			RHBORB_unguard(0)

			cb(impl,0,&addrSeq);

			return;
		}
	}

#ifdef HAVE_SOCKADDR_IN6
	{
		struct sockaddr_in6 addr_in6;
		int i;
		memset(&addr_in6,0,sizeof(addr_in6));

		i=inet_pton(AF_INET6,somThis->name,&addr_in6.sin6_addr);

		if (i > 0)
		{
			_IDL_SEQUENCE_SOMD_NetworkAddress addrSeq={1,1,NULL};
			SOMD_NetworkAddress addr={NULL,AF_INET6,
							{sizeof(addr_in6),
							sizeof(addr_in6),NULL}};
			addr.value._buffer=(octet *)&addr_in6;
			addrSeq._buffer=&addr;

			addr_in6.sin6_family=AF_INET6;
#ifdef HAVE_SOCKADDR_SA_LEN
			addr_in6.sin6_len=sizeof(addr_in6);
#endif
			addr_in6.sin6_port=htons(somThis->port);

			RHBORB_unguard(0)

			cb(impl,0,&addrSeq);

			return;
		}
	}
#endif

#ifdef USE_THREADS
	RHBResolver_AddRef(somSelf);
	somThis->task.start=rhbbsdr_thread;
	somThis->task.param=somSelf;
	somThis->task.failed=rhbbsdr_thread_failed;
	RHBORB_StartThreadTask(impl->orb,&somThis->task);
#else
	if (somThis->name)
	{
		Sockets SOMSTAR socks=somThis->protocol->impl->socketObject;
		Environment ev;

		SOM_InitEnvironment(&ev);

		if (socks)
		{
			struct hostent *hp=Sockets_somsGethostbyname(
						socks,
						&ev,
						somThis->name);

			if (hp && (ev._major==NO_EXCEPTION))
			{
				_IDL_SEQUENCE_SOMD_NetworkAddress addr={0,0,NULL};
				build_addrSeq(&addr,hp,somThis->port);
				cb(impl,0,&addr);

				SOMD_FreeType(&ev,&addr,somdTC_sequence_SOMD_NetworkAddress);

				cb=NULL;
			}
		}

		SOM_UninitEnvironment(&ev);
	}

	if (cb && impl) cb(impl,SOMDERROR_HostName,NULL);
#endif

	RHBORB_unguard(0)
}

static int RHBLINK rhbbsdnr_Release(RHBResolverRef somSelf)
{
RHBResolverData *somThis=RHBProtocolGetData(somSelf);
RHBImplementationDef *impl=NULL;

	if (somd_atomic_dec(&somThis->lUsage))
	{
		return 0;
	}

	RHBORB_guard(0)

	if (somThis->protocol->impl->resolvers)
	{
		if (somThis==somThis->protocol->impl->resolvers)
		{
			somThis->protocol->impl->resolvers=somThis->next;
		}
		else
		{
			RHBResolverData *o=somThis->protocol->impl->resolvers;

			while (o->next != somThis)
			{
				o=o->next;

				if (!o) break;
			}

			if (o)
			{
				o->next=somThis->next;
			}
		}
	}

	impl=somThis->impl;
	somThis->impl=NULL;

	RHBORB_unguard(0)

	if (impl)
	{
		RHBImplementationDef_Release(impl);
	}

	if (somThis->protocol)
	{
		RHBProtocol_Release(somThis->protocol);
	}

	RHBOPT_ASSERT(!somThis->lUsage)

#ifdef USE_RESOLVER_CHILD
	if (somThis->dataSeq._buffer)
	{
		SOMFree(somThis->dataSeq._buffer);
	}
#endif

	SOMFree(somSelf);

	return 0;
}

static int RHBLINK rhbbsdnr_QueryInterface(RHBResolver *somSelf,void *refidd,void **ppv)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(refidd)
	RHBOPT_unused(ppv)

	return -1;
}

static int RHBLINK rhbbsdnr_AddRef(RHBResolver * somSelf)
{
RHBResolverData *somThis=RHBResolverGetData(somSelf);

	somd_atomic_inc(&somThis->lUsage);

	return 0;
}

#if 0
static void rhbbsd_dump_addr(struct sockaddr *addr)
{
#ifdef RHBSocket_dump
	struct sockaddr_in *in;
	int host;
	short port;

	in=(struct sockaddr_in *)addr;

	host=in->sin_addr.s_addr;
	host=ntohl(host);
	port=in->sin_port;
	port=ntohs(port);
/*	somPrintf("(%x:%d)",(int)host,(int)port);*/
#endif
	RHBOPT_unused(addr)
}
#endif

static void RHBLINK rhbbsd_dump(RHBSocketRef somSelf)
{
#ifdef RHBSocket_dump
	RHBSocketData *somThis=somSelf->impl;

	if (somThis->fd != INVALID_SOCKET)
	{
		struct sockaddr_in addr;
		int j;
		octet *op;

		j=sizeof(addr);
		addr.sin_port=0;
		addr.sin_addr.s_addr=0;

		getsockname(somThis->fd,(struct sockaddr *)&addr,&j);
		op=(void *)&addr.sin_addr.s_addr;
		somPrintf("(%d.%d.%d.%d:%d)",
				(int)op[0],
				(int)op[1],
				(int)op[2],
				(int)op[3],
				(int)htons(addr.sin_port));

		somPrintf("--");

		j=sizeof(addr);
		addr.sin_port=0;
		addr.sin_addr.s_addr=0;

		getpeername(somThis->fd,(struct sockaddr *)&addr,&j);
		op=(void *)&addr.sin_addr.s_addr;
		somPrintf("(%d.%d.%d.%d:%d)",
				(int)op[0],
				(int)op[1],
				(int)op[2],
				(int)op[3],
				(int)htons(addr.sin_port));
	}
	else
	{
		somPrintf("(INVALID_SOCKET)");
	}
	somPrintf("\n");
#endif
	RHBOPT_unused(somSelf)
}

#ifdef USE_THREADS
struct rhbbsdr_thread
{
	RHBResolverRef ref;
	RHBImplementationDef *impl;

#ifdef USE_RESOLVER_CHILD
	_IDL_SEQUENCE_octet addrBuf;
	any addrSeqAny;
#else
		_IDL_SEQUENCE_SOMD_NetworkAddress addrSeq;
	#ifdef HAVE_GETADDRINFO
	/* preferred option */
		struct addrinfo *ai;
		char *buf;	
		struct hostent he;
	#else
	/* required for IPV6 */
		#ifdef HAVE_GETIPNODEBYNAME
			struct hostent *hp;
		#else
		/* last resort, gethostbyname_r */
			struct RHBProtocol_hostent he;
		#endif
	#endif
#endif
};
RHBOPT_cleanup_begin(rhbbsdr_thread_cleanup,pv)

	struct rhbbsdr_thread *data=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);

#ifdef USE_RESOLVER_CHILD
	SOMD_FreeType(&ev,&data->addrSeqAny,TC_any);
#else
	#ifdef HAVE_GETADDRINFO
		if (data->buf) SOMFree(data->buf);
		if (data->ai) freeaddrinfo(data->ai);
	#else
		#ifdef HAVE_GETIPNODEBYNAME
			if (data->hp) freehostent(data->hp);
		#endif
	#endif
		SOMD_FreeType(&ev,&(data->addrSeq),somdTC_sequence_SOMD_NetworkAddress);
#endif

	SOM_UninitEnvironment(&ev);

	if (data->ref) RHBResolver_Release(data->ref);
	if (data->impl) RHBImplementationDef_Release(data->impl);

RHBOPT_cleanup_end

static void SOMLINK rhbbsdr_thread(void *a,Environment *ev)
{
#ifdef USE_RESOLVER_CHILD
	SOCKET pair[2]={INVALID_SOCKET,INVALID_SOCKET};
#endif
	RHBSocketError e=0;
	struct rhbbsdr_thread data={NULL,NULL,
		{0,0,NULL}};
#ifdef USE_RESOLVER_CHILD
#else
	#ifdef HAVE_GETADDRINFO
		int error_num=-1;
		data.ai=NULL;
		data.buf=NULL;
	#else
		#ifdef HAVE_GETIPNODEBYNAME
			int error_num=-1;
			data.hp=NULL;
		#else
			struct hostent *hp=NULL;
			memset(&data.he,0,sizeof(data.he));
		#endif
	#endif
#endif
		data.ref=a;
		RHBOPT_cleanup_push(rhbbsdr_thread_cleanup,&data);

#ifdef USE_RESOLVER_CHILD
	if (!socketpair(AF_UNIX,SOCK_STREAM,0,pair))
	{
		struct RHBResolverData *somThis=data.ref->impl;

		close_on_exec(pair[0]);
		close_on_exec(pair[1]);

		if (!set_non_blocking(pair[0]))
		{
			char *name=somThis->name;
#ifdef _WIN32
			DWORD tid=0;
			int i=strlen(name)+1;
			struct dnr_async *dnr=malloc(sizeof(*dnr)+i);
			dnr->fd=pair[1];
			dnr->name=(char *)(dnr+1);
			dnr->port=somThis->port;
			dnr->data._length=0;
			dnr->data._maximum=0;
			dnr->data._buffer=NULL;
			memcpy(dnr->name,name,i);
			somThis->hThread=CreateThread(NULL,0,dnr_async_r,dnr,0,&tid);
			if (somThis->hThread)
			{
				somThis->fd=pair[0];
				pair[0]=INVALID_SOCKET;
				pair[1]=INVALID_SOCKET;
			}
			else
			{
				free(dnr);
			}
#else
			struct dnr_async dnr=DNR_ASYNC_INIT;
			dnr.fd=pair[1];
			dnr.name=name;
			dnr.port=somThis->port;
			FORK_STATUS
			somThis->pid=pth_fork();
			if (somThis->pid)
			{
				if (somThis->pid < 0)
				{
					FORK_STATUS
					somThis->pid=0;
				}
				else
				{
					FORK_STATUS
					somThis->fd=pair[0];
					pair[0]=INVALID_SOCKET;
				}
			}
			else
			{
				soclose(pair[0]);
				FORK_STATUS
				_exit(dnr_async(&dnr));
			}
#endif
		}

FORK_STATUS
		if (pair[0] != INVALID_SOCKET) soclose(pair[0]);
		if (pair[1] != INVALID_SOCKET) soclose(pair[1]);

		while ((somThis->fd != INVALID_SOCKET) && somThis->packet_size)
		{
			fd_set fdr;
			FD_ZERO(&fdr);
			FD_SET(somThis->fd,&fdr);

			FORK_STATUS

			if (pth_select(somThis->fd+1,&fdr,NULL,NULL,NULL))
			{
				FORK_STATUS

				if (somThis->fd==INVALID_SOCKET) break;

				if (FD_ISSET(somThis->fd,&fdr))
				{
					char buf[256];
					char *p=buf;
					int len=somThis->packet_size;

					if (somThis->length_bytes_len < sizeof(somThis->length_bytes))
					{
						p=somThis->length_bytes+somThis->length_bytes_len;
						len=sizeof(somThis->length_bytes)-somThis->length_bytes_len;
					}
					else
					{
						if (len > sizeof(buf))
						{
							len=sizeof(buf);
						}
					}
					
					len=recv(somThis->fd,p,len,0);

					if (len < 0)
					{
#ifdef _WIN32
						int e=WSAGetLastError();
						if (e!=WSAEWOULDBLOCK)
#else
						int e=errno;
						if (e!=EWOULDBLOCK)
#endif
						{
							break;
						}
					}
					else
					{
						if (!len) break;

						if (somThis->length_bytes_len < sizeof(somThis->length_bytes))
						{
							somThis->length_bytes_len+=len;

							if (somThis->length_bytes_len==sizeof(somThis->length_bytes))
							{
								if (sizeof(somThis->length_bytes)==sizeof(somThis->packet_size))
								{
									memcpy(&(somThis->packet_size),
											somThis->length_bytes,
											sizeof(somThis->packet_size));
								}
								else
								{
									int i=0;
									memcpy(&i,somThis->length_bytes,sizeof(i));
									somThis->packet_size=i;
								}

								somThis->dataSeq._maximum=somThis->packet_size;
								if (somThis->packet_size)
								{
									somThis->dataSeq._buffer=SOMMalloc(somThis->packet_size);
									if (!somThis->dataSeq._buffer) break;
								}
							}
						}
						else
						{
							dataSeq_add(&somThis->dataSeq,buf,len);
							somThis->packet_size-=len;
						}
					}
				}
			}
		}

FORK_STATUS

		somPrintf("dnr received %d bytes\n",somThis->dataSeq._length);

		if (somThis->fd!=INVALID_SOCKET)
		{
			SOCKET fd=somThis->fd;
			somThis->fd=INVALID_SOCKET;
					FORK_STATUS
			soclose(fd);
		}

#ifdef _WIN32
		if (somThis->hThread)
		{
			WaitForSingleObject(somThis->hThread,INFINITE);
			CloseHandle(somThis->hThread);
			somThis->hThread=NULL;
		}
#else
		if (somThis->pid)
		{
			int s=0;
			FORK_STATUS
			waitpid(somThis->pid,&s,0);
			FORK_STATUS
			somThis->pid=0;
		}
#endif
		if (somThis->dataSeq._length)
		{
			Environment ev;
			RHBORB *orb=somThis->protocol->impl->orb;
			SOM_InitEnvironment(&ev);
			data.addrSeqAny=IOP_Codec_decode_value(orb->iop_codec,&ev,
				&somThis->dataSeq,somdTC_sequence_SOMD_NetworkAddress);
			SOM_UninitEnvironment(&ev);
		}
	}
#else
	#ifdef HAVE_GETADDRINFO
			error_num=getaddrinfo(data.ref->impl->name,NULL,NULL,&data.ai);

			if (error_num)
			{
				e=error_num;
			}
			else
			{
				struct addrinfo *ai=data.ai;
				
				while (ai)
				{
					if (do_include(ai))
					{
						data.addrSeq._maximum++;
					}

					ai=ai->ai_next;
				}

				if (data.addrSeq._maximum)
				{
					size_t len=data.addrSeq._maximum*sizeof(data.addrSeq._buffer[0]);
					data.addrSeq._buffer=SOMMalloc(len);
					if (data.addrSeq._buffer) memset(data.addrSeq._buffer,0,len);
				}

				if (data.addrSeq._buffer)
				{
					SOMD_NetworkAddress *addr=data.addrSeq._buffer;

					ai=data.ai;

					while (ai)
					{
						if (do_include(ai))
						{
							addr->family=ai->ai_family;
							addr->name=somd_dupl_string(ai->ai_canonname);

							addr->value._maximum=(long)ai->ai_addrlen;
							addr->value._length=(long)ai->ai_addrlen;
							addr->value._buffer=SOMMalloc(ai->ai_addrlen);
							memcpy(addr->value._buffer,ai->ai_addr,ai->ai_addrlen);

							switch (ai->ai_addr->sa_family)
							{
							case AF_INET:
								{
									struct sockaddr_in *sin4=(void *)addr->value._buffer;
									sin4->sin_port=htons(data.ref->impl->port);
								}
								break;
	#ifdef HAVE_SOCKADDR_IN6
							case AF_INET6:
								{
									struct sockaddr_in6 *sin6=(void *)addr->value._buffer;
									sin6->sin6_port=htons(data.ref->impl->port);
								}
								break;
	#endif
							}

							addr++;
							data.addrSeq._length++;
						}

						ai=ai->ai_next;
					}
				}
			}
	#else
		#ifdef HAVE_GETIPNODEBYNAME
			#if defined(AF_INET6)
				data.hp=getipnodebyname(data.ref->impl->name,AF_INET6,AI_DEFAULT,&error_num);
			#else
				data.hp=getipnodebyname(data.ref->impl->name,AF_INET,AI_DEFAULT,&error_num);
			#endif
			if (data.hp) build_addrSeq(&data.addrSeq,hp,data.ref->impl->port);
		#else
			hp=rhbbsdp_GetHostByName_r(data.ref->impl->protocol,
										data.ref->impl->name,
										&data.he);
			if (hp) build_addrSeq(&data.addrSeq,hp,data.ref->impl->port);
		#endif
	#endif
#endif

	RHBORB_guard(orb)
	data.impl=data.ref->impl->impl;
	data.ref->impl->impl=NULL;
	RHBORB_unguard(orb)

	if (data.impl)
	{
#ifdef USE_RESOLVER_CHILD
		if (data.addrSeqAny._value)
		{
			data.ref->impl->cb(data.impl,e,data.addrSeqAny._value);
		}
#else
		if (data.addrSeq._length)
		{
			data.ref->impl->cb(data.impl,e,&data.addrSeq);
		}
#endif
		else
		{
			data.ref->impl->cb(data.impl,SOMDERROR_HostName,NULL);
		}
	}

	RHBOPT_cleanup_pop();
}

static void SOMLINK rhbbsdr_thread_failed(void *pv)
{
	RHBResolverRef ref=pv;
	RHBImplementationDef *impl;

	debug_somPrintf(("resolver thread spawn failed\n"));

	RHBORB_guard(orb)
	impl=ref->impl->impl;
	ref->impl->impl=0;
	RHBORB_unguard(orb)

	if (impl)
	{
		ref->impl->cb(impl,SOMDERROR_CouldNotStartThread,NULL);
	}

	RHBResolver_Release(ref);
	if (impl) RHBImplementationDef_Release(impl);
}
#endif

static void RHBLINK rhbbsdnr_Cancel(RHBResolverRef ref)
{
	RHBResolverData *somThis=ref->impl;
	RHBImplementationDef *impl;
	RHBORB_guard(orb)
	impl=somThis->impl;
	somThis->impl=NULL;
	RHBORB_unguard(orb)
	if (impl) RHBImplementationDef_Release(impl);
}

static boolean RHBLINK rhbbsd_IsOneShot(RHBSocketRef sock)
{
#ifdef USE_ONESHOTIO
	return 1;
#else
	return 0;
#endif
}

static void RHBLINK rhbbsdp_atfork_child(RHBProtocolRef somSelf)
{
	struct RHBProtocolData *somThis=somSelf->impl;
	struct RHBSocketData *sock=somThis->socket_impl_list;
	struct RHBResolverData *res=somThis->resolvers;
#ifdef USE_THREADS
	struct thread_select *thr=somThis->threads;
#else
	Sockets SOMSTAR socketObject=somThis->socketObject;
	Environment ev=RHBSOMD_EnvironmentInit;
#endif
	
	while (sock)
	{
		if (sock->fd != INVALID_SOCKET)
		{
#ifdef _PLATFORM_UNIX_
#	ifdef USE_THREADS
			close(sock->fd);
#	else
			Sockets_somsSoclose(socketObject,&ev,sock->fd);
#	endif
			sock->fd=INVALID_SOCKET;
#endif
		}

		sock=sock->next;
	}

#ifdef USE_THREADS
	while (thr)
	{
		if (thr->fd_signal_read != INVALID_SOCKET)
		{
#ifdef _PLATFORM_UNIX_
			close(thr->fd_signal_read);
			thr->fd_signal_read=INVALID_SOCKET;
#endif
		}

		if (thr->fd_signal_write != INVALID_SOCKET)
		{
#ifdef _PLATFORM_UNIX_
			close(thr->fd_signal_write);
			thr->fd_signal_write=INVALID_SOCKET;
#endif
		}

		thr=thr->next;
	}
#endif

	while (res)
	{
#ifdef USE_RESOLVER_CHILD
		if (res->fd != INVALID_SOCKET)
		{
#ifdef _PLATFORM_UNIX_
			close(res->fd);
			res->fd=INVALID_SOCKET;
#endif
		}
#endif

		res=res->next;
	}
}
