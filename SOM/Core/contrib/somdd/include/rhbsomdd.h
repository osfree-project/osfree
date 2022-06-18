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

#ifdef _WIN32
	#ifdef HAVE_WINSOCK2_H
		#include <winsock2.h>
	#else
		#include <winsock.h>
	#endif
	#ifdef HAVE_WS2IP6_H
		#include <ws2ip6.h>
	#endif
	#ifdef HAVE_WS2TCPIP_H
		#include <ws2tcpip.h>
	#endif
	/* this stops somdd.exp & somdd.lib */
	#ifdef WIN32_DLLEXPORT
		#undef WIN32_DLLEXPORT
	#endif
	#define WIN32_DLLEXPORT
#endif

#include <sys/types.h>
#include <stdio.h>

#ifdef _PLATFORM_UNIX_
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <sys/ioctl.h>
#	include <sys/wait.h>
#	include <unistd.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <netdb.h>
#	include <errno.h>
#	include <pwd.h>
#	include <signal.h>
#	include <fcntl.h>
#	include <time.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
	#include <Processes.h>
	#ifdef USE_THREADS
		#include <rhbpthrd.h>
	#endif
#else
	#ifndef _WIN32
		#ifndef SOCKET
			#define SOCKET int
		#endif
		#ifndef INVALID_SOCKET
			#define INVALID_SOCKET  ((SOCKET)-1)
		#endif
	#endif
#endif

#ifdef HAVE_STRINGS_H
#	include <strings.h>
#endif

#include <rhbsomex.h>
#include <somref.h>
#include <snglicls.h>
#include <somir.h>
#include <containd.h>
#include <containr.h>
#include <repostry.h>
#include <corba.h>

#undef Repository_lookup_id
#undef Contained__get_id

#ifdef USE_THREADS
#else
	#include <emtypes.h>
	#include <eman.h>
	#include <emregdat.h>
#endif

#include <somd.h>
#include <somuutil.h>
#include <rhbsomkd.h>
#include <rhbexecd.h>
#include <poa.h>

typedef struct RHBDynamicServerObject RHBDynamicServerObject;
typedef struct RHBRequestList RHBRequestList;
typedef struct RHBServerRequestList RHBServerRequestList;

typedef struct RHBDynamicServerObjectList
{
	RHBDynamicServerObject *list;
} RHBDynamicServerObjectList;

typedef struct RHBProcess
{
	struct RHBProcessVtbl *lpVtbl;
} RHBProcess;

#define RHBProcess_AddRef(p)    (p)->lpVtbl->AddRef(p)
#define RHBProcess_Release(p)   (p)->lpVtbl->Release(p)
#define RHBProcess_Equal(p,q)	(p)->lpVtbl->Equal(p,q)
#define RHBProcess_Print(p)		(p)->lpVtbl->Print(p)
#ifdef USE_THREADS
#	define RHBProcess_Kill(p,m)		(p)->lpVtbl->Kill(p,m)
#else
#	define RHBProcess_Kill(p)		(p)->lpVtbl->Kill(p)
#endif

#ifdef _PLATFORM_MACINTOSH_
RHBProcess *RHBProcessNew(ProcessSerialNumber *psn);
RHBProcess *RHBProcessFind(ProcessSerialNumber *psn);
#endif

typedef struct tagRHBServerPatient
{
	struct tagRHBServerPatient *next;
	struct tagRHBServerWaitingRoom *room;
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_t event;
	#else	
		#ifdef _WIN32
			HANDLE h_event;
		#else
			struct mthread_cond_t event;
		#endif
	#endif
#endif
} RHBServerPatient;

typedef struct tagRHBServerWaitingRoom
{
	RHBServerPatient *list;
} RHBServerWaitingRoom;

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		typedef pthread_mutex_t *SOMDD_MutexRef;
	#else
		#ifdef _WIN32
			typedef HANDLE SOMDD_MutexRef;
		#else
			typedef struct mthread_mutex_t *SOMDD_MutexRef;
		#endif
	#endif
#endif

#ifdef USE_THREADS
void SOMDD_WaitInWaitingRoom(RHBServerWaitingRoom *room,SOMDD_MutexRef);
void SOMDD_NotifyWaitingRoom(RHBServerWaitingRoom *room,SOMDD_MutexRef);
#define SOMDD_AcquireGlobalMutex()   SOMDD_AcquireGlobalMutexDebug(__FILE__,__LINE__)
#ifdef SOMDD_AcquireGlobalMutex
SOMDD_MutexRef SOMDD_AcquireGlobalMutexDebug(const char *,int);
#else
SOMDD_MutexRef SOMDD_AcquireGlobalMutex();
#endif
void SOMDD_ReleaseGlobalMutex(SOMDD_MutexRef);
#endif

#define SOMDD_FLAGS_DEBUG     1

extern boolean SOMDD_running;

struct SOMDD_daemon;

struct SOMDD_message
{
	struct SOMDD_message *next;
	octet *ptr;
	unsigned long length;
	octet data[1];
};

#define USE_LINKED_RHBEXECD
#if defined(USE_PTHREADS) || !defined(USE_THREADS)
#	define USE_SELECT_RHBPROCESSMGR
#endif

#ifdef USE_THREADS
#else
struct emanEvent
{
	SOMEEMRegisterData SOMSTAR regData;
	SOMEClientEvent SOMSTAR event;
	long token;
	int sig;
	boolean queued;
};
#define emanEventInit	{NULL,NULL,0,0,0}
struct emanSink
{
	struct RHBProcessMgr *mgr;
	SOMEEMRegisterData SOMSTAR regData;
	long token;
	long mask;
} emanSinks;
#define emanSinkInit	{NULL,NULL,0,0}
#endif

struct RHBProcessMgrChild
{
#ifdef _WIN32
			PROCESS_INFORMATION psinfo;
#	ifdef USE_LINKED_RHBEXECD
#		ifdef USE_SELECT_RHBPROCESSMGR
			SOCKET fdServerRead,fdServerWrite;
#			define RHBPROCESSMGRCHILD_INIT_SOCK   INVALID_SOCKET,INVALID_SOCKET
#		else /* USE_SELECT_RHBPROCESSMGR */
			HANDLE hServerRead,hServerWrite;
#			define RHBPROCESSMGRCHILD_INIT_SOCK   INVALID_HANDLE_VALUE,INVALID_HANDLE_VALUE
#		endif /* USE_SELECT_RHBPROCESSMGR */
			HANDLE hServerConsole;
#	endif /* !_USE_LINKED_RHBEXECD */
#	define RHBPROCESSMGRCHILD_INIT_BEGIN			\
					{0,0,0,0},						\
					RHBPROCESSMGRCHILD_INIT_SOCK,INVALID_HANDLE_VALUE
#else /* _WIN32 */
			pid_t pidServer;
			SOCKET fdServerRead,fdServerWrite,fdServerError;
#	define RHBPROCESSMGRCHILD_INIT_BEGIN   0,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET
#endif /* _WIN32 */
#ifdef USE_SELECT_RHBPROCESSMGR
#	ifdef USE_THREADS
#		ifdef USE_PTHREADS
			pthread_t tidSelect;
			pthread_cond_t eventSelect;
#		endif /* USE_PTHREADS */
#		define RHBPROCESSMGRCHILD_INIT_END
#	else /* USE_THREADS */
		SOMEEMan SOMSTAR emanObject;
		struct emanEvent quitEvent,termEvent,hupEvent;
		struct emanSink sinks[2];
#		define RHBPROCESSMGRCHILD_INIT_END								\
					,NULL,emanEventInit,emanEventInit,emanEventInit,	\
					{emanSinkInit,emanSinkInit}
#	endif /* USE_THREADS */
#else /* USE_SELECT_RHBPROCESSMGR */
#	define RHBPROCESSMGRCHILD_INIT_END
#endif /* USE_SELECT_RHBPROCESSMGR */
};
typedef struct RHBProcessMgrChild RHBProcessMgrChild;

#define RHBPROCESSMGRCHILD_INIT   { RHBPROCESSMGRCHILD_INIT_BEGIN  RHBPROCESSMGRCHILD_INIT_END }

typedef struct RHBProcessMgr 
{
	struct RHBProcessData *running;
	struct RHBProcessData *zombies;
	struct SOMDD_message *outgoing;
	struct SOMDD_message *incoming;
	ImplRepository SOMSTAR implrep;
	struct RHBProcessMgrChild *childInfo;
	boolean bQuitWrite;
	int header_len;
	octet header[4];
#ifdef USE_SELECT_RHBPROCESSMGR
	SOCKET fdClientRead,fdClientWrite;		/* talking to exec proc */
#	ifdef USE_THREADS
	SOCKET fdSignalRead,fdSignalWrite;		/* signalling select */
#		define RHBPROCESSMGR_INIT_FDSIGNAL	,INVALID_SOCKET,INVALID_SOCKET
#	else
#		define RHBPROCESSMGR_INIT_FDSIGNAL
#	endif
#else /* USE_SELECT_RHBPROCESSMGR */
#	if defined(_WIN32)
	HANDLE hWriteThread,hWriteEvent;
	HANDLE hClientRead,hClientWrite;
#	else /* _WIN32 */
#		error unknown platform
#	endif /* _WIN32 */
#endif /* USE_SELECT_RHBPROCESSMGR */
} RHBProcessMgr;

#define RHBPROCESSMGR_INIT_COMMON   \
	NULL,NULL,NULL,NULL,			\
	NULL,NULL,0,0,{'A','B','C','D'}

#ifdef USE_SELECT_RHBPROCESSMGR
#	define RHBPROCESSMGR_INIT		\
	INVALID_SOCKET,INVALID_SOCKET RHBPROCESSMGR_INIT_FDSIGNAL
#endif

/* life cycle APIs */
int RHBProcessMgr_init(RHBProcessMgr *);
void RHBProcessMgr_term(RHBProcessMgr *);

/* around the dispatch loop */
int RHBProcessMgr_begin(RHBProcessMgr *,Environment *ev);
void RHBProcessMgr_end(RHBProcessMgr *);

RHBProcess *RHBProcessNew(
	  RHBProcessMgr *,
	  Environment *ev,
	  char *programname,
	  char *impl_id,
	  char *alias
#ifdef USE_THREADS
	  ,SOMDD_MutexRef mutex
#endif
	  );

struct RHBProcessVtbl
{
	int (*QueryInterface)(RHBProcess *,void *,void **);
	int (*AddRef)(RHBProcess *);
	int (*Release)(RHBProcess *);
	int (*Equal)(RHBProcess *,RHBProcess *);
	int (*Kill)(RHBProcess *
#ifdef USE_THREADS
		,SOMDD_MutexRef
#endif
		);
	int (*Print)(RHBProcess *);
};

extern int SOMDD_main(struct SOMDD_daemon *,
					  RHBProcessMgr *,
					  int argc,char **argv);

extern char *SOMDD_get_repos_filename(int argc,char **argv);

#ifndef SOM_Module_implreps_Source
#define SOM_Module_implreps_Source
#endif

#ifndef SOM_Module_somddevm_Source
#define SOM_Module_somddevm_Source
#endif

#include <rhbsomut.h>
#include <rhbsomkd.h>

#ifdef Contained
	#ifndef CORBA_Contained
		#define CORBA_Contained SOMObject
	#endif
	#ifndef CORBA_InterfaceDef
		#define CORBA_InterfaceDef SOMObject
	#endif
#endif


#include <implreps.h>
#include <somddevm.h>

#ifdef _PLATFORM_MACINTOSH_
void SOMDD_failed(const char *);
#endif

extern char *SOMDD_dupl_string(const char *p);
void SOMDD_impl_failed(RHBProcess *proc,char *impl_id,ORBStatus rc);
boolean SOMDD_is_impl_alive_in(char *impl_id,RHBProcess *proc);
char *SOMDD_ior_from_data(ImplementationDef_octet_seqP data);

/* used by WIN32 NT service and Mac Gestalt */

void SOMDD_up(struct SOMDD_daemon *);
void SOMDD_down(struct SOMDD_daemon *);

struct SOMDD_msg
{
	int type;
#ifdef _WIN32
	unsigned long id;
#endif
	const char *text;
};

#define SOMDD_LOG_OK		1
#define SOMDD_LOG_INFO		2
#define SOMDD_LOG_WARN		3
#define SOMDD_LOG_ERROR		4

#ifdef _WIN32
	#define SOMDD_DEFMSG(n,s,i,t) static struct SOMDD_msg n={SOMDD_LOG_##s,i,t};
#else
	#define SOMDD_DEFMSG(n,s,i,t) static struct SOMDD_msg n={SOMDD_LOG_##s,t};
#endif

void SOMDD_syslog(struct SOMDD_msg *,...);

/*
#define SOMDD_LOGMSG_FAILED_ACTIVATE	1
#define SOMDD_LOGMSG_FAILED_PUBLISH		2
#define SOMDD_LOGMSG_INTERRUPT			3
#define SOMDD_LOGMSG_READY				4
#define SOMDD_LOGMSG_SHUTDOWN			5
#define SOMDD_LOGMSG_DISPATCH			6
*/

#ifdef _WIN32
extern long SOMDD_init_security(void);
extern BOOL SOMDD_AllocConsole(void);
#endif

#ifdef _PLATFORM_MACINTOSH_
extern void SOMDD_someWait(unsigned long delay_ms);
#endif

/* this is used by Mac to set type/creator on prefs file */
void SOMDD_prepare_repos_file(char *f);

#ifdef _WIN32
	DWORD WINAPI SOMDD_wait_for_child_end(void *pv);
	#ifdef AF_UNIX
	int __stdcall socketpair(int family,int type,int protocol,SOCKET *result);
	#endif
#endif

extern somToken somddTC__IIOP_ProfileBody_1_0,
				somddTC__IIOP_ProfileBody_1_1,
				somddTC__IOP_IOR,
				somddTC__PortableServer_ForwardRequest,
				somddTC__SOMDServer,
				somddTC__CORBA_InterfaceDef;

#define somddTC_IIOP_ProfileBody_1_0			(TypeCode)(void *)&(somddTC__IIOP_ProfileBody_1_0)
#define somddTC_IIOP_ProfileBody_1_1		    (TypeCode)(void *)&(somddTC__IIOP_ProfileBody_1_1)
#define somddTC_IOP_IOR							(TypeCode)(void *)&(somddTC__IOP_IOR)
#define somddTC_PortableServer_ForwardRequest   (TypeCode)(void *)&(somddTC__PortableServer_ForwardRequest)
#define somddTC_SOMDServer						(TypeCode)(void *)&(somddTC__SOMDServer)
#define somddTC_CORBA_InterfaceDef				(TypeCode)(void *)&(somddTC__CORBA_InterfaceDef)

IOP_Codec SOMSTAR	SOMDD_create_codec(Environment *ev);
IOP_TaggedProfile SOMDD_create_IIOP_tagged_profile(IOP_Codec SOMSTAR codec,Environment *ev,IIOP_ProfileBody_1_1 *body);
_IDL_SEQUENCE_octet SOMDD_create_IOP_IOR(IOP_Codec SOMSTAR codec,Environment *ev,IOP_IOR *ior);
_IDL_SEQUENCE_octet *SOMDD_create_initial_ior(IOP_Codec SOMSTAR codec,Environment *ev,unsigned short iiop_port,char *impl_id);
ImplementationDef SOMSTAR SOMDD_create_initial_ImplDef(Environment *ev);
SOMObject SOMSTAR SOMDD_get_obj_from_ior(_IDL_SEQUENCE_octet *seq,Environment *ev,TypeCode tcInterface);

struct RHBDynamicServerObject
{
	struct RHBDynamicServerObject *next;
	RHBDynamicServerObjectList *server_object_list;

	RHBRequestList *request_list;
	RHBServerRequestList *server_request_list;

	SOMObject SOMSTAR dynobj;
	SOMObject SOMSTAR somref;

	ImplRepositoryServer_ImplRepContents *impl;
	IOP_Codec SOMSTAR codec;
};

#define RHBServerRequest_state_wait_for_up		1	/* wait until server is up				*/
#define RHBServerRequest_state_is_up			2	/* server is up							*/
#define RHBServerRequest_state_begin_interface	3	/* invoke(_interface)					*/
#define RHBServerRequest_state_wait_interface	4	/* wait...								*/
#define RHBServerRequest_state_begin_describe	5	/* invoke(_get_id,&&_get_somModifiers)	*/
#define RHBServerRequest_state_wait_describe	6	/* wait...								*/
#define RHBServerRequest_state_begin_reply		7	/* begin replying						*/
#define RHBServerRequest_state_wait_reply		8	/* in process of replying				*/
#define RHBServerRequest_state_end				9	/* complete								*/
#define RHBServerRequest_state_delete			10	/* too late								*/

struct RHBRequest
{
	struct RHBRequest *next;
	Request SOMSTAR request;
	struct RHBServerRequest *server_request;
	NamedValue result;
	RHBRequestList *request_list;
	void (*complete)(struct RHBRequest *req,Environment *,somToken mtx);
	boolean cancelled,pending;
};

struct RHBServerRequest
{
	struct RHBServerRequest *next;
	SOMObject SOMSTAR dynobj;
	SOMObject SOMSTAR somref;
	SOMObject SOMSTAR client_proxy;
	SOMObject SOMSTAR server_request;
	SOMObject SOMSTAR codec;
	InterfaceDef SOMSTAR interface_def;
	ReferenceData object_key;
	Identifier type_id;
	_IDL_SEQUENCE_somModifier mods;
	RHBRequestList *request_list;
	RHBServerRequestList *server_request_list;
	ImplRepositoryServer_ImplRepContents *impl;
	long state;
	struct RHBRequest _interface,_get_id,_get_somModifiers;
	Environment ev;
};

struct RHBServerRequestList
{
	struct RHBServerRequest *list;
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_t p_event;
	#else
		HANDLE h_event;
	#endif
#endif
};

struct RHBRequestList
{
	struct RHBRequest *list;
	long lSequence;
	long lNested;
#ifdef USE_THREADS
	SOMObject SOMSTAR somSelf;
	int isAlive;
	int doQuit;
	#ifdef USE_PTHREADS
		pthread_t tid;
		pthread_cond_t p_event,p_dead;
	#else
		DWORD tid;
		HANDLE h_thread,h_event,h_dead;
	#endif
#endif
};

#ifdef _WIN32
	#include <somddmsg.h>
#endif

#if defined(_PLATFORM_MACINTOSH_) 
	#define DONT_USE_SIGNALS
	#define sigset_t    aargh do not use
	#define sigaction  aargh do not use
	#if 0
		#ifdef RHBSOM_Trace
			#undef RHBSOM_Trace
		#endif
		#define RHBSOM_Trace(p)  { somPrintf("%s:%d, %s\n",__FILE__,__LINE__,p); }
	#endif
#endif

#ifdef _WIN32
	#if (!defined(USE_PTHREADS)) && (defined(USE_THREADS) || defined(_WIN32S))
		#define DONT_USE_SIGNALS
		#define sigset_t    aargh do not use
		#define sigaction  aargh do not use
		#define signal	  aargh do not use
	#endif
#endif

char * SOMLINK Contained__get_id(SOMObject SOMSTAR,Environment *);
SOMObject SOMSTAR SOMLINK Repository_lookup_id(Repository SOMSTAR,Environment *ev,char *);


#ifndef SOMOA_somDuplicateReference
#define SOMOA_somDuplicateReference(x)	SOMRefObject_somDuplicateReference(x)
#endif

#ifndef SOMOA_somRelease
#define SOMOA_somRelease(x)	SOMRefObject_somRelease(x)
#endif

#ifndef ImplRepository_somDuplicateReference
#define ImplRepository_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef SOMDObject_somDuplicateReference
#define SOMDObject_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef SOMDD_Server_somDuplicateReference
#define SOMDD_Server_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef ImplRepositoryServer_somDuplicateReference
#define ImplRepositoryServer_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef SOMDClientProxy_somDuplicateReference
#define SOMDClientProxy_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef ImplementationDef_somDuplicateReference
#define ImplementationDef_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef IOP_Codec_somDuplicateReference
#define IOP_Codec_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef DynamicServerObject_somDuplicateReference
#define DynamicServerObject_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

#ifndef SOMDObject_somRelease
#define SOMDObject_somRelease(x) SOMRefObject_somRelease(x)
#endif

#ifndef SOMEClientEvent_somRelease
#define SOMEClientEvent_somRelease(x) SOMRefObject_somRelease(x)
#endif

#ifndef ImplementationDef_somRelease
#define ImplementationDef_somRelease(x) SOMRefObject_somRelease(x)
#endif

#ifndef SOMEEMRegisterData_somRelease
#define SOMEEMRegisterData_somRelease(x) SOMRefObject_somRelease(x)
#endif
