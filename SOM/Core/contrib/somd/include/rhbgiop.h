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

#ifndef __RHBOPT_H__
	#error <rhbopt.h> should have been included
#endif

#ifndef __RHBGIOP_H__
#define __RHBGIOP_H__

#include <time.h>
#include <stdlib.h>

#ifdef _PLATFORM_WIN32_
	/* #define DEBUG_MEMLEAK*/
#endif

#ifdef _PLATFORM_MACINTOSH_
#	ifndef BUILD_SOMTC
		/* #define DEBUG_MEMLEAK*/
#	endif
#endif

#include <unotypes.h>

#ifdef USE_SELECT
#	include <emtypes.h>
#	include <event.h>
#	include <eventmsk.h>
#	include <soms.h>
#endif

#include <somtc.h>
#include <somtcnst.h>
#include <somd.h>
#include <stdio.h>

#define GIOP_LIMIT_XFER  ((sizeof(int) > 2) ? 0xf000 : 0x7000)

/* all decisions should have been made, let's validate them */

#ifdef USE_SELECT
	#ifdef USE_THREADS
		#error Cannot have 'select' and threads
	#endif
	#ifdef USE_PTHREADS
		#error Cannot have 'select' and pthreads
	#endif
#else
	#ifndef USE_THREADS
		#define USE_THREADS
		/* #define USE_PTHREADS*/
	#endif
#endif

#ifdef _PLATFORM_UNIX_
	#include <sys/types.h>
	#include <netdb.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <sys/time.h>
#endif

#ifndef INVALID_SOCKET
	#ifdef _PLATFORM_MACINTOSH_
		#include <Processes.h>
		/* these are for OpenTransport */
		/*#define INVALID_SOCKET  kOTInvalidEndpointRef
		typedef EndpointRef     SOCKET;*/
		#ifdef USE_SELECT
			typedef long SOCKET;
			#define INVALID_SOCKET  ((SOCKET)(-1))
		#endif
	#else
		/* this should be hauled in for UNIX */
		typedef int SOCKET;
		#define INVALID_SOCKET  ((SOCKET)(-1))
	#endif
#endif

typedef struct RHBInterfaceDef RHBInterfaceDef;
typedef struct RHBRequestList RHBRequestList;
typedef struct RHBGIOPRequestStream RHBImplementationDef;
typedef struct RHBEventManager RHBEventManager;

typedef struct RHBContained RHBContained;
typedef struct RHBSocket RHBSocket;
typedef RHBSocket * RHBSocketRef;
typedef struct RHBProtocol RHBProtocol;
typedef RHBProtocol * RHBProtocolRef;
typedef struct RHBResolver RHBResolver;
typedef RHBResolver * RHBResolverRef;

#ifdef USE_THREADS
typedef void (SOMLINK * RHBORB_ThreadFunction)(void *,Environment *ev);
typedef void (SOMLINK * RHBORB_ThreadFailFunction)(void *);
struct RHBORB_thread_task
{
	struct RHBORB_thread_task *next;
	RHBORB_ThreadFunction start;
	RHBORB_ThreadFailFunction failed;
	void *param;
};
#endif

#define USE_RHBSOCKET

#ifndef RHBSOMD_must_be_guarded
	#define RHBSOMD_must_be_guarded    /* call a routine to confirm guarding in debug */
#endif

#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
		#ifdef _WIN32
			#ifdef HANDLE
				#undef HANDLE
			#endif
			#define HANDLE		   	      dont use HANDLE
			#ifdef CRITICAL_SECTION
				#undef CRITICAL_SECTION
			#endif
			#define CRITICAL_SECTION     dont use CRITICAL_SECTION
/*			#ifdef WaitForSingleObject
				#undef WaitForSingleObject
			#endif
			#define WaitForSingleObject   dont use WaitForSingleObject*/
		#endif
	#endif
#endif /* USE_THREADS */

typedef ORBStatus (*RHBORB_operation_handler)
	(struct RHBContained *op,
	Environment *ev,
	any *result);

SOMEXTERN void RHBRequestList_init(RHBRequestList *);

struct RHBRequestList
{
	struct RHBRequestListVtbl *lpVtbl;
	RHBRequest *first;
	RHBRequest *last;
};

struct RHBRequestListVtbl
{
	void (*add)(struct RHBRequestList *,struct RHBRequest *);
	boolean (*remove)(struct RHBRequestList *,struct RHBRequest *);
	boolean (*empty)(struct RHBRequestList *);
	struct RHBRequest * (*pop)(struct RHBRequestList *);
};

#define RHBRequestList_empty(a)		(a)->lpVtbl->empty((a))
#define RHBRequestList_pop(a)		(a)->lpVtbl->pop((a))
#define RHBRequestList_add(a,b)		(a)->lpVtbl->add((a),b)
#define RHBRequestList_remove(a,b)	(a)->lpVtbl->remove((a),b)

#ifdef __cplusplus
#error this is a C only file
#endif

#ifdef USE_SELECT
	#include <eman.h>
	typedef void (*RHBEventCallback)(Environment *ev,SOCKET sock,int mask,void /*struct RHBSocket*/ *refCon);
	typedef struct tag_RHBSocketEvent
	{
		void /*struct RHBSocket*/ *refCon;
		SOMEEMRegisterData SOMSTAR regData;
		long read_token,write_token,except_token;
		SOCKET fd;
		RHBEventCallback pfn;
		struct tag_RHBSocketEvent *next;
		int event_mask;
#ifdef _PLATFORM_MACINTOSH_
		ProcessSerialNumber psn;
#endif
		RHBORB *c_orb;
	} RHBSocketEvent;
#endif

SOMEXTERN void SOMLINK RHBEventManager_init(
			RHBEventManager *somThis,RHBORB *orb);

struct RHBEventManager
{
	struct RHBEventManagerVtbl *lpVtbl;
	RHBORB *orb;
#ifdef USE_SELECT
	RHBSocketEvent *eventList;
	SOMEEMan SOMSTAR user_eman;
	Sockets SOMSTAR socketObject;
	SOMD_Callback *user_callback;
	long async_proc_token;
	char *async_proc_type;
#ifdef _PLATFORM_MACINTOSH_
	QHdr osQ;
	ProcessSerialNumber psn;
#endif
#endif
};

#if !defined(RHBOPT_SHARED_DATA) && !defined(USE_THREADS)
#	ifdef SOMDLLEXPORT
		SOMDLLEXPORT 
#	endif
extern Sockets SOMSTAR SOMD_SocketObject;
#endif

struct RHBAsyncEvent;

struct RHBEventManagerVtbl
{
	void (*uninit)(struct RHBEventManager *);
#ifdef USE_SELECT
	void (*handle_one)(struct RHBEventManager *,Environment *ev,unsigned long);
	RHBSocketEvent *(*add)(struct RHBEventManager *,SOCKET,RHBEventCallback,void *);
	void (*remove)(struct RHBEventManager *,RHBSocketEvent *);
	void (*set_mask)(struct RHBEventManager *,RHBSocketEvent *,int);
	void (*move_to_end)(struct RHBEventManager *,RHBSocketEvent *);
	SOMObject SOMSTAR (*async_init)(struct RHBEventManager *eman,Environment *,struct RHBAsyncEvent *);
#else
#endif
};

#ifdef USE_SELECT
	#define RHBEventManager_handle_one(a,b,c)		(a)->lpVtbl->handle_one((a),b,c)
	#define RHBEventManager_add(a,b,c,d)			(a)->lpVtbl->add((a),b,c,d)
	#define RHBEventManager_remove(a,b)				(a)->lpVtbl->remove((a),b)
	#define RHBEventManager_set_mask(a,b,c)			(a)->lpVtbl->set_mask((a),b,c)
	#define RHBEventManager_move_to_end(a,b)		(a)->lpVtbl->move_to_end((a),b)
#endif

#ifdef USE_SELECT
	struct RHBAsyncEvent
	{
		struct RHBAsyncEventVtbl *lpVtbl;
#ifdef _PLATFORM_MACINTOSH_
		QElem qElem;
		QHdrPtr qHdr;
#endif
		RHBORB *orb;
		void *refcon;
		void (*callback)(struct RHBAsyncEvent *);
		SOMObject SOMSTAR clientEvent;
	};

	struct RHBAsyncEventVtbl
	{
		void (*post)(struct RHBAsyncEvent *);
		void (*uninit)(struct RHBAsyncEvent *);
	};

	#define RHBAsyncEvent_post(a)		(a)->lpVtbl->post(a)
	#define RHBAsyncEvent_uninit(a)		(a)->lpVtbl->uninit(a)
	#define RHBAsyncEvent_init(a,b,c)  	c->events.lpVtbl->async_init(&c->events,b,a)
#endif

typedef struct RHBORB_sockaddr
{
	union
	{
		struct sockaddr generic;
		struct sockaddr_in ipv4;
#ifdef HAVE_SOCKADDR_IN6
		struct sockaddr_in6 ipv6;
#endif
	} u;
} RHBORB_sockaddr;

typedef struct RHBORB_ThreadTask
{
#ifdef USE_THREADS
	struct RHBORB_ThreadTask *next;
	RHBORB *orb_ptr;
	struct RHBORB_thread_task *task;
	struct RHBORB_killer *killer;
#ifdef USE_PTHREADS
	pthread_t pThread;
	pthread_cond_t pEvent;
#else
	HANDLE hThread;
	HANDLE hEvent;
	DWORD dwThread;
#endif
#endif

	RHBServerRequest *server_request;
} RHBORB_ThreadTask;

typedef struct SOMD_Globals SOMD_Globals;

struct SOMD_GlobalsVtbl
{
	void (*Init)(struct SOMD_Globals *,Environment *);
	void (*Uninit)(struct SOMD_Globals *,Environment *);
#ifdef RHBOPT_SHARED_DATA
	void (*Destroy)(struct SOMD_Globals *);
#endif
};

struct SOMD_Globals
{
	struct SOMD_GlobalsVtbl *lpVtbl;
	boolean initialised;
	rhbatomic_t Init_count;
	RHBORB *orb;

#ifdef USE_THREADS
		long numThreads;
	#ifdef USE_PTHREADS
		pthread_cond_t p_event;
		pthread_key_t p_thread_key;
	#else
		HANDLE h_event;
		DWORD thread_key;
	#endif
#endif

#ifdef RHBOPT_SHARED_DATA
	struct SOMD_Globals *next;
	#ifdef _PLATFORM_MACINTOSH_
		ProcessSerialNumber psn;
	#else
		#ifdef _WIN32
			DWORD tid;
		#else
			int pid;
		#endif
	#endif
	SOMObject SOMSTAR ORBObject;
	SOMObject SOMSTAR ImplRepObject;
	SOMObject SOMSTAR ImplDefObject;
	SOMObject SOMSTAR ServerObject;
	SOMObject SOMSTAR SOMOAObject;
	SOMObject SOMSTAR DefaultContext;
	SOMObject SOMSTAR ObjectMgr;
	long DebugFlag,RecvBufferSize,TraceLevel;
#endif
};

#ifdef RHBOPT_SHARED_DATA
	extern SOMD_Globals *resolve_somd_globals(void);
	#define somd_globals (*(resolve_somd_globals()))
#else
	extern SOMD_Globals somd_globals;
#endif

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#define RHBSOMD_set_thread_specific(x) \
			pthread_setspecific(somd_globals.p_thread_key,(void *)x)
		#ifdef HAVE_PTHREAD_GETSPECIFIC_STD
			#define RHBSOMD_get_thread_specific(x) \
				x=pthread_getspecific(somd_globals.p_thread_key)
		#else
			#define RHBSOMD_get_thread_specific(x) \
				{ pthread_addr_t pv; \
					if (pthread_getspecific(somd_globals.p_thread_key,&pv)) \
					{ x=NULL; } else { x=(void *)pv; } }
		#endif
	#else
		#define RHBSOMD_set_thread_specific(x) \
			TlsSetValue(somd_globals.thread_key,x)
		#define RHBSOMD_get_thread_specific(x) \
			x=TlsGetValue(somd_globals.thread_key)
	#endif
#endif

struct RHBORB_ThreadTask * SOMLINK RHBSOMD_get_thread(void);

#define RHBORB_hex_char(a,b)				(a)->lpVtbl->hex_char(a,b)
#define RHBORB_create_hex_string(a,b,c)		(a)->lpVtbl->create_hex_string(a,b,c)
#define RHBORB_dump_sequence(a,b)			(a)->lpVtbl->dump_sequence(a,b)
#define RHBORB_read_hex(a,b)				(a)->lpVtbl->read_hex(a,b)

/* Mac is only using cooperative threads, others are preemptive */
#if defined(USE_THREADS) && !defined(_PLATFORM_MACINTOSH_)
	#ifdef USE_PTHREADS
		#define somd_atomic_inc(pl)			rhbatomic_inc(pl)
		#define somd_atomic_dec(pl)			rhbatomic_dec(pl)
	#else
		#define somd_atomic_inc(x)	(void)InterlockedIncrement(x)
		#define somd_atomic_dec(x)	InterlockedDecrement(x)
	#endif
#else
	#define somd_atomic_inc(pl)      (void)(++(*pl))
	#define somd_atomic_dec(pl)      (--(*pl))
#endif

typedef struct RHBORB_request_pool_t
{
	RHBRequestList completed;
	boolean waiting;
	unsigned long outstanding;
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_t *event;
	#else
		HANDLE event;
	#endif

#endif
} RHBORB_request_pool_t;

SOMEXTERN RHBORB *SOMLINK RHBORBNew(struct SOMD_Globals *globals,Environment *ev);

#define RHBORB_AddRef(x)								(x)->lpVtbl->AddRef(x)
#define RHBORB_Release(x)								(x)->lpVtbl->Release(x)
#define RHBORB_shutdown(x,e)							(x)->lpVtbl->shutdown(x,e)
#define RHBORB_Close(x,e)								(x)->lpVtbl->Close(x,e)
#define RHBORB_set_server_requests_enabled(a,b)			(a)->lpVtbl->set_server_requests_enabled(a,b)
#define RHBORB_server_state_changed(a)					(a)->lpVtbl->server_state_changed(a)
#define RHBORB_add_interface(x,i,o)						(x)->lpVtbl->add_interface(x,i,o)
#define RHBORB_remove_interface(x,i,o)					(x)->lpVtbl->remove_interface(x,i,o)
#define RHBORB_get_object(x,y,a,b,c,d,e)				(x)->lpVtbl->get_object(x,y,a,b,c,d,e)
#define RHBORB_get_object_from_ior(x,y,a,b,c,d)			(x)->lpVtbl->get_object_from_ior(x,y,a,b,c,d)
#define RHBORB_get_local_server(x,y,z)					(x)->lpVtbl->get_local_server(x,y,z)
#define RHBORB_get_server_impl(x,y,z)					(x)->lpVtbl->get_server_impl(x,y,z)
#define RHBORB_get_exception_tc(x,y,z)					(x)->lpVtbl->get_exception_tc(x,y,z)
#define RHBORB_get_contained(x,y,z)						(x)->lpVtbl->get_contained(x,y,z)
#define RHBORB_get_interface(x,y,z)						(x)->lpVtbl->get_interface(x,y,z)
#define RHBORB_get_principal(x,y)						(x)->lpVtbl->get_principal(x,y)
#define RHBORB_get_SOMDServer_type_id(x,y)				(x)->lpVtbl->get_SOMDServer_type_id(x,y)
#define RHBORB_object_to_string(x,y,z)					(x)->lpVtbl->object_to_string(x,y,z)
#define RHBORB_string_to_object(x,y,z)					(x)->lpVtbl->string_to_object(x,y,z)
#define RHBORB_create_proxy_class(a,b,c,d,e)			(a)->lpVtbl->create_proxy_class(a,b,c,d,e)
#define RHBORB_acquire_interface_repository(a,b,c)		(a)->lpVtbl->acquire_interface_repository(a,b,c)
#define RHBORB_release_interface_repository(somThis,ev,rep) if (rep) { Repository_somRelease(rep); }
#define RHBORB_get_impl(a,b,c)							(a)->lpVtbl->get_impl(a,b,c)
#define RHBORB_add_impl(a,b)							(a)->lpVtbl->add_impl(a,b)
#define RHBORB_remove_impl(a,b)							(a)->lpVtbl->remove_impl(a,b)
#define RHBORB_convert_class_to_interface(a,b)			(a)->lpVtbl->convert_class_to_interface(a,b)
#define RHBORB_convert_interface_to_class(a,b)			(a)->lpVtbl->convert_interface_to_class(a,b)
#define RHBORB_is_oidl(a,b)								(a)->lpVtbl->is_oidl(a,b)
#define RHBORB_is_method_oidl(a,b,c)					(a)->lpVtbl->is_method_oidl(a,b,c)
#define RHBORB_any_client_references(a,b)				(a)->lpVtbl->any_client_references(a,b)
#define RHBORB_get_proxy_class(a,b,c)					(a)->lpVtbl->get_proxy_class(a,b,c)
#define RHBORB_leading_colons(a,b)						(a)->lpVtbl->leading_colons(a,b)
#define RHBORB_id_to_abs(a,b)							(a)->lpVtbl->id_to_abs(a,b)
#define RHBORB_abs_to_id(a,b,c,d)						(a)->lpVtbl->abs_to_id(a,b,c,d)
#define RHBORB_get_SOMDPORT(a)							(a)->lpVtbl->get_SOMDPORT(a)
#define RHBORB_get_HOSTNAME(a)							(a)->lpVtbl->get_HOSTNAME(a)
#define RHBORB_get_somdd_implid(a)						(a)->lpVtbl->get_somdd_implid(a)

#ifdef USE_THREADS
	#define RHBORB_wait_server_state_changed(a)				(a)->lpVtbl->wait_server_state_changed(a)
	#define RHBORB_StartThreadTask(a,b)						(a)->lpVtbl->StartThreadTask(a,b)
#else
	SOMEXTERN RHBServerRequest * RHBORB_pop_first_queued_request(
			RHBORB *somThis);
	SOMEXTERN void RHBORB_ProcessEvents(RHBORB *somThis,Environment *ev);
#endif

#ifdef USE_THREADS
	extern struct rhbmutex_t somd_mutex;

#	define RHBORB_guard(o)				rhbmutex_lock(&somd_mutex);
#	define RHBORB_unguard(o)			rhbmutex_unlock(&somd_mutex);

#	define somd_is_guarded()			rhbmutex_locked(&somd_mutex)

	#ifdef USE_PTHREADS
#		define somd_wait(e,f,l)				rhbmutex_wait(&somd_mutex,~0UL,(e))
#		define somd_timed_wait(e,s,f,l)		rhbmutex_wait(&somd_mutex,(s),(e))
	#else
#		define somd_wait(e,f,l)				rhbmutex_wait(&somd_mutex,~0UL,(e))
#		define somd_timed_wait(e,s,f,l)		rhbmutex_wait(&somd_mutex,(s),(e))
	#endif
	void RHBGIOPORB_dump_threads(void);
#else
	#define RHBORB_guard(a)
	#define RHBORB_unguard(a)
#endif

#ifdef _WIN32
int somd_DllMain(void *pv,int reason,void*extra);
#endif

#define RHBORB_newImplDef(_somThis)			SOMClass_somNew(_somThis->somClass_ImplementationDef_ref)
#define RHBORB_newRequest(_somThis)			SOMClass_somNew(_somThis->somClass_Request_ref)
#define RHBORB_isSOMDObject(_somThis,_ref)	(SOMObject_somIsA(_ref,_somThis->somClass_SOMDObject_ref) ? _ref : NULL)
#define RHBORB_isRequest(_somThis,_ref)		(SOMObject_somIsA(_ref,_somThis->somClass_Request_ref) ? _ref : NULL)

#ifdef USE_SELECT
	#define RHBORB_idle(_somThis,ev,flags)						RHBEventManager_handle_one((&_somThis->events),ev,flags)
	#define RHBORB_remove_socket_event(_somThis,tok)			RHBEventManager_remove((&_somThis->events),tok)
	#define RHBORB_add_socket_event(_somThis,sock,pfn,refCon)	RHBEventManager_add((&_somThis->events),sock,pfn,refCon)
	#define RHBORB_set_socket_mask(_somThis,token,mask)			RHBEventManager_set_mask((&_somThis->events),token,mask)
#endif

struct RHBORBVtbl
{
	void (*AddRef)(RHBORB *);
	void (*Release)(RHBORB *);
	void (*Close)(RHBORB *,Environment *);
	void (*shutdown)(RHBORB *,Environment *);
	void (*add_interface)(RHBORB *somThis,RHBInterfaceDef *ifnew,boolean orb_locked);
	void (*remove_interface)(RHBORB *somThis,RHBInterfaceDef *ifnew,boolean orb_locked);
	SOMDObject SOMSTAR (*get_object)(RHBORB *,Environment *,SOMClass SOMSTAR,RHBImplementationDef *,RHBInterfaceDef *,_IDL_SEQUENCE_octet *,TypeCode);
	SOMDObject SOMSTAR (*get_object_from_ior)(RHBORB *,Environment *,IOP_IOR *,RHBImplementationDef *,SOMClass SOMSTAR,TypeCode);
	Repository SOMSTAR (*acquire_interface_repository)(RHBORB *somThis,Environment *ev,char *why);
	SOMDServer SOMSTAR (*get_local_server)(RHBORB *somThis,Environment *ev,char *impl_id);
	RHBGIOPRequestStream * (*get_server_impl)(RHBORB *somThis,Environment *ev,char *impl_id);
	TypeCode (*get_exception_tc)(RHBORB *orb,exception_type type,char *id);
	RHBContained *(*get_contained)(RHBORB *,Environment *,char *);
	RHBInterfaceDef * (*get_interface)(RHBORB *somThis,Environment *ev,char * name);
	RHBImplementationDef * (*get_impl)(RHBORB *somThis,Environment *ev,IIOP_ProfileBody_1_0 *iop);
	Principal SOMSTAR (*get_principal)(RHBORB *somThis,Environment *ev);
	char *(*get_SOMDServer_type_id)(RHBORB *somThis,Environment *ev);
	char *(*object_to_string)(RHBORB *somThis,Environment *ev,SOMDObject SOMSTAR object);
	SOMDObject SOMSTAR (*string_to_object)(RHBORB *somThis,Environment *ev,corbastring str);
	SOMClass SOMSTAR (*create_proxy_class)(RHBORB *orb,Environment *ev,SOMClassMgr SOMSTAR clsMgr,SOMClass SOMSTAR proxy_class,SOMClass SOMSTAR local_class);
	char (*hex_char)(RHBORB *,octet);
	char *(*create_hex_string)(RHBORB *,void *,size_t);
	void (*dump_sequence)(RHBORB *,_IDL_SEQUENCE_octet *st);
	_IDL_SEQUENCE_octet (*read_hex)(RHBORB *,char *);
	void (*add_impl)(RHBORB *,RHBGIOPRequestStream *);
	void (*remove_impl)(RHBORB *,RHBGIOPRequestStream *);
	SOMClass SOMSTAR (*get_proxy_class)(RHBORB *,Environment *,char *);
	char * (*convert_class_to_interface)(RHBORB *somThis,SOMClass SOMSTAR cls);
	char * (*convert_interface_to_class)(RHBORB *somThis,char *interface_name);
	boolean (*set_server_requests_enabled)(RHBORB *somThis,boolean enabled);
	void (*server_state_changed)(RHBORB *somThis);
	boolean (*is_oidl)(RHBORB *orb,char *clsName);
	boolean (*is_method_oidl)(RHBORB *orb,SOMObject SOMSTAR target,char * id);
	boolean (*any_client_references)(RHBORB *orb,SOMObject SOMSTAR obj);
	char * (*leading_colons)(RHBORB *,char *p);
	char * (*id_to_abs)(RHBORB *,const char *id);
	char * (*abs_to_id)(RHBORB *,const char *abs,long major,long minor);
	char * (*get_HOSTNAME)(RHBORB *);
	unsigned short (*get_SOMDPORT)(RHBORB *);
	char * (*get_somdd_implid)(RHBORB *);
#ifdef USE_THREADS
	void (*wait_server_state_changed)(RHBORB *);
	void (*StartThreadTask)(RHBORB *somThis,struct RHBORB_thread_task *);
#else
#endif
};

struct RHBORB
{
	struct RHBORBVtbl *lpVtbl;
	struct SOMD_Globals *globals;
	RHBProtocolRef protocol;

	long closing;
	Principal SOMSTAR principal;

	RHBSOMUT_KeyDataSet contained_by_abs;
	rhbatomic_t lUsage;
	RHBEventManager events;

	RHBInterfaceDef *iface;

	RHBSOMUT_KeyDataSet ifaces_by_id;
	RHBSOMUT_KeyDataSet ifaces_by_abs;

	RHBGIOPRequestStream *impls;

	RHBORB_request_pool_t pool;

	struct
	{
		boolean request_loop_ready;
#ifdef USE_THREADS
		struct RHBORB_server_state_waiter *change_waiters;
#endif
	} state;

	SOMClass SOMSTAR somClass_ImplementationDef_ref;
	SOMClass SOMSTAR somClass_Request_ref;		 
	SOMClass SOMSTAR somClass_SOMDObject_ref;		 
	SOMClass SOMSTAR somClass_SOMDClientProxy_ref; 
	SOMClass SOMSTAR somClass_SOMOA_ref;
	SOMClass SOMSTAR somClass_SOMDServer_ref;
	SOMClass SOMSTAR somClass_Context_ref;
	SOMClass SOMSTAR somClass_Principal_ref;
	SOMClass SOMSTAR somClass_NVList_ref;
	SOMClass SOMSTAR somClass_somStream_MemoryStreamIO_ref;
	SOMClass SOMSTAR somClass_PortableServer_DynamicImpl_ref;
	SOMClass SOMSTAR somClass_SOMDServerStub_ref;
	SOMClass SOMSTAR somClass_ServerRequest_ref;

	SOMObject SOMSTAR iop_codec;
	SOMObject SOMSTAR iop_codecFactory;

	somId somId_get_impl_id,
		  somId_get_impl_hostname,
		  somId_get_somModifiers,
		  somId_get_absolute_name,
		  somId_get_id,
		  somIdRepository,
		  somIddescribe;

#ifdef USE_THREADS
	struct RHBORB_ThreadTask *threads;
	struct
	{
		boolean running;
		struct RHBORB_thread_task *tasks;
#ifdef USE_PTHREADS
		pthread_cond_t pEvent;
		pthread_t pThread;
#else
		HANDLE hEvent,hThread;
		DWORD tid;
#endif
		struct RHBORB_killer *killer;
		struct RHBORB_ThreadTask *dead_list;
	} child;
#else
	RHBORB_ThreadTask thread_globals;
#endif
};

typedef struct RHBSendData
{
	struct RHBSendData *next;
	octet *seekPtr;
	unsigned long toGo;
	octet _buffer[1];
} RHBSendData;

RHBGIOPRequestStream *RHBGIOPRequestStreamNew(
		RHBORB *the_orb,
		IIOP_ProfileBody_1_0 *connection);

#define RHBImplementationDef_AddRef(x)						(x)->lpVtbl->AddRef(x)
#define RHBImplementationDef_Release(x)						(x)->lpVtbl->Release(x)
#define RHBImplementationDef_interrupt_closed(x,e)			(x)->lpVtbl->interrupt_closed(x,e)
#define RHBImplementationDef_write_GIOP_msg(x,e,v,t,d)		(x)->lpVtbl->write_GIOP_msg(x,e,v,t,d)
#define RHBImplementationDef_add_outstanding(x,y)			(x)->lpVtbl->add_outstanding(x,y)
#define RHBImplementationDef_remove_outstanding(x,y)		(x)->lpVtbl->remove_outstanding(x,y)
#define RHBImplementationDef_get_wrapper(x,y,z)				(x)->lpVtbl->get_wrapper(x,y,z)
#define RHBImplementationDef_begin_listen(x,y,z)			(x)->lpVtbl->begin_listen(x,y,z)
#define RHBImplementationDef_end_listen(x,y)				(x)->lpVtbl->end_listen(x,y)
#define RHBImplementationDef_remove_client_references(x)	(x)->lpVtbl->remove_client_references(x)
#define RHBImplementationDef_acquire_somdServer(x,y)		(x)->lpVtbl->acquire_somdServer(x,y)
#define RHBImplementationDef_get_target_object(x,y,z,a)		(x)->lpVtbl->get_target_object(x,y,z,a)
#define RHBImplementationDef_add_client_ref(x,y)			(x)->lpVtbl->add_client_ref(x,y)
#define RHBImplementationDef_remove_client_ref(x,y,z)		(x)->lpVtbl->remove_client_ref(x,y,z)
#define RHBImplementationDef_allocate_sequence(x)			(x)->lpVtbl->allocate_sequence(x)
#define RHBImplementationDef_queue_request(x,y,z)			(x)->lpVtbl->queue_request(x,y,z)
#define RHBImplementationDef_notify_somdd(x,y,z)			(x)->lpVtbl->notify_somdd(x,y,z)
#define RHBImplementationDef_debug_printf(x,y)				(x)->lpVtbl->debug_printf(x,y)

void SOMLINK RHBSOMD_notify_somdd_impl_using_ipc(
		RHBORB *somThis,
		Environment *ev,
		short transition,
		char *impl_id,
		char *impl_server_ior,
		ORBStatus rc);

typedef struct RHBImplClientRef
{
	SOMObject SOMSTAR somobj;
	rhbatomic_t lUsage;
} RHBImplClientRef;

struct RHBGIOPRequestStream_sink
{
	struct RHBSocketSinkJumpTable *vtbl;
	struct RHBGIOPRequestStream *outer;
	rhbatomic_t lUsage;
};

struct RHBSocketListener
{
	/* allows a listener to have multiple sockets
		to listen on,
		typical UNIX IPV6 implementations allow
		the same socket to be used for both
		ipv4 and ipv6 traffic, not so with _WIN32
		so we're going to add another via this mechanism
	*/
	struct RHBSocketSinkJumpTable *vtbl;
	struct RHBSocketListenerVtbl *vtbl2;
	struct RHBServerData *server;
	struct RHBSocketListener *next;
	RHBORB *orb;
	rhbatomic_t lUsage;
	RHBSocketRef fd;
	unsigned short port;
};

struct RHBSocketListenerVtbl
{
	boolean (*begin_listen)(
		struct RHBSocketListener *,
		struct RHBServerData *server,
		short family,struct sockaddr *,int len);
	void (*end_listen)(struct RHBSocketListener *);
};

struct RHBSocketListener *RHBSocketListenerNew(RHBORB *orb);

struct RHBServerData
{
	struct RHBServerDataVtbl *lpVtbl;
	struct RHBServerData *next;
	SOMDServer SOMSTAR somdServer;
	SOMOA SOMSTAR somoa;
	ImplementationDef SOMSTAR implDef;
	boolean is_ready;
	rhbatomic_t lUsage;
	boolean somdd_notified_state;
	struct RHBGIOPRequestStream *listener;
	rhbatomic_t deactivating; /* when deactivating it */
	struct RHBSocketListener *listener_list;
	_IDL_SEQUENCE_octet impl_id; /* in begin listen */
};

#define impl_is_deactivating(x)   (((x)->server.data) ? ((x)->server.data->deactivating) : 1)

struct RHBServerDataVtbl
{
	void (*AddRef)(struct RHBServerData *);
	void (*Release)(struct RHBServerData *);
};

typedef struct RHBServerData RHBServerData;

struct RHBGIOPRequestStreamVtbl
{
	void (*AddRef)(struct RHBGIOPRequestStream *);
	void (*Release)(struct RHBGIOPRequestStream *);
	void (*interrupt_closed)(struct RHBGIOPRequestStream *,Environment *ev);
	void (*write_GIOP_msg)(struct RHBGIOPRequestStream *somThis,Environment *ev,GIOP_Version *version,GIOP_MsgType type,_IDL_SEQUENCE_octet *data);
	void (*add_outstanding)(RHBGIOPRequestStream *somThis,RHBServerRequest *req);
	void (*remove_outstanding)(RHBGIOPRequestStream *somThis,RHBServerRequest *req);
	ImplementationDef SOMSTAR (*get_wrapper)(RHBGIOPRequestStream *somThis,Environment *ev,char *id_hint);
	void (*begin_listen)(RHBGIOPRequestStream *somThis,Environment *ev,char *impl_id);
	void (*end_listen)(RHBGIOPRequestStream *somThis,Environment *ev);
	void (*remove_client_references)(RHBGIOPRequestStream *somThis);
	SOMDServer SOMSTAR (*acquire_somdServer)(RHBGIOPRequestStream *somThis,Environment *ev);
	SOMObject SOMSTAR (*get_target_object)(RHBGIOPRequestStream *somThis,Environment *ev,_IDL_SEQUENCE_octet *object_key,SOMDServer SOMSTAR *ps);
	void (*add_client_ref)(RHBGIOPRequestStream *somThis,SOMObject SOMSTAR somobj);
	boolean (*remove_client_ref)(RHBGIOPRequestStream *somThis,SOMObject SOMSTAR somobj,boolean already_guarded);
	unsigned long (*allocate_sequence)(RHBGIOPRequestStream *somThis);
	ORBStatus (*queue_request)(RHBGIOPRequestStream *somThis,Environment *ev,RHBRequest *request);
	void (*notify_somdd)(RHBGIOPRequestStream *somThis,Environment *ev,boolean up);
	void (*debug_printf)(RHBGIOPRequestStream *somThis,Environment *ev);
};

struct RHBGIOPRequestStream
{
	struct RHBGIOPRequestStreamVtbl *lpVtbl;
	rhbatomic_t lUsage;
	RHBRequestList pending;
	RHBRequestList executing_normal;
	RHBRequestList executing_oneway;
	
	int already_reading;

	struct
	{
		RHBSendData *head;
		RHBSendData *tail;
	} transmitting;

	_IDL_SEQUENCE_octet receiving;

	struct
	{
		unsigned long _maximum;
		unsigned long _length;
		RHBImplClientRef *_buffer;
	}	client_references;

	boolean receiving_header;
/*	any receive_header;*/
	GIOP_MessageHeader GIOP_header;
	boolean receive_doSwap;
	RHBServerRequest *server_requests;
#ifdef USE_SELECT
	RHBServerRequest *queued_request_list;
	RHBServerRequest *active_server_request;
#endif
	RHBGIOPRequestStream *next;
	RHBORB *orb;
	boolean is_closing;	/* used to indicate when it can't do any more... */
	boolean deleting;	/* used to indicate final clear up in progress */

	struct
	{
		char * host;
		unsigned short port;
		_IDL_SEQUENCE_SOMD_NetworkAddress addrSeq;
		unsigned long index;
	} address;

	struct
	{
		RHBSocket *fd;
		int connect_attempts;

		RHBResolver *resolver;

		boolean connected;
		boolean connecting;
		boolean readable;
		boolean writeable;
/*		boolean write_posted;*/
		boolean is_listener;
		boolean is_server;
		boolean transmitting;
	} connection;

	struct
	{
		unsigned long nextSequence;
	} transport;

	struct RHBGIOPRequestStream_sink sink;

	struct
	{
		int release_on_close;

		RHBORB_sockaddr addr;

		struct 
		{
			RHBORB_sockaddr addr;
		} client;

		RHBServerData *data;
	} server;

	RHBServerRequest *outstanding_request_list;

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_t p_EventSem;
	#else
		HANDLE h_EventSem;
	#endif
#endif

	SOMCDR_unmarshal_filter unmarshal_filter;
};

SOMEXTERN RHBInterfaceDef * RHBInterfaceDefNew(
					RHBORB *the_orb,
					char * the_name,
					boolean orb_locked);

#define RHBInterfaceDef_AddRef(x)				(x)->lpVtbl->AddRef(x)
#define RHBInterfaceDef_Release(x)				(x)->lpVtbl->Release(x)
#define RHBInterfaceDef__get_usage(x)			((x)->lUsage)
#define RHBInterfaceDef_get_proxy_class(x,e)	(x)->lpVtbl->get_proxy_class(x,e)
#define RHBInterfaceDef_get_absolute_name(x,e)	(x)->lpVtbl->get_absolute_name(x,e)

struct RHBInterfaceDefVtbl
{
	void (*AddRef)(struct RHBInterfaceDef *);
	void (*Release)(struct RHBInterfaceDef *);
	char *(*get_absolute_name)(RHBInterfaceDef *,Environment *);
	SOMClass SOMSTAR (*get_proxy_class)(RHBInterfaceDef *somThis,Environment *ev);
};

struct RHBInterfaceDef
{
	struct RHBInterfaceDefVtbl *lpVtbl;
	RHBORB *orb;
	rhbatomic_t lUsage;
	RHBInterfaceDef *next;
	char *absolute_name;
/*	SOMClass SOMSTAR somClass__Proxy_ref;*/
	char id[1];
};

SOMEXTERN void RHBObject_somInit(
					RHBObject *somThis,
					RHBInterfaceDef *def,
					RHBGIOPRequestStream *impl,
					_IDL_SEQUENCE_octet *o_key);


struct RHBContained
{
	struct RHBContainedVtbl *lpVtbl;
	rhbatomic_t lUsage;
	RHBSOMUT_KeyData key;
	RHBORB *orb;
	any desc_value;
	_IDL_SEQUENCE_somModifier modifiers;
	unsigned short describeFlags,modifierFlags;
	RHBORB_operation_handler handler;
	char *id;
	char absolute_name[1];
};

struct RHBContainedVtbl
{
	void (*AddRef)(RHBContained *);
	void (*Release)(RHBContained *);
	char *(*get_id)(RHBContained *,Environment *ev);
};

#define RHBContained_AddRef(a)					a->lpVtbl->AddRef(a)
#define RHBContained_Release(a)					a->lpVtbl->Release(a)
#define RHBContained_get_id(a,e)				a->lpVtbl->get_id(a,e)

#define RHBObject_get_server_object(x,y,z)			(x)->lpVtbl->get_server_object(x,y,z)
#define RHBObject_somDumpSelfInt(x,y,z)				(x)->lpVtbl->somDumpSelfInt(x,y,z)
#define RHBObject_create_request(a,b,c,d,e,f,g,h,i) (a)->lpVtbl->create_request(a,b,c,d,e,f,g,h,i)
#define RHBObject_change_location(x,y,z)			(x)->lpVtbl->change_location(x,y,z)
#define RHBObject_somUninit(x)						if ((x)->lpVtbl) (x)->lpVtbl->somUninit(x)
#define RHBObject_get_type_id(x,y)					(x)->lpVtbl->get_type_id(x,y)

struct RHBObjectVtbl
{
	void (*somUninit)(RHBObject *somThis);
	SOMObject SOMSTAR (*get_server_object)(RHBObject *obj,Environment *ev,_IDL_SEQUENCE_octet *n);
	void (*somDumpSelfInt)(RHBObject *somThis,SOMDObject SOMSTAR somSelf,long level);
	ORBStatus (*create_request)(
					RHBObject *somThis,
					SOMDObject SOMSTAR somobj,
					Environment *ev,
					_IDL_SEQUENCE_string * ctx,
					Identifier operation,
					NVList SOMSTAR arg_list,
					NamedValue *result,
					RHBRequest **request,
					Flags flags);
	void (*change_location)(
			RHBObject *somThis,
			Environment *ev,
			RHBObject *obj);
	char * (*get_type_id)(RHBObject *somThis,Environment *ev);
};

struct RHBObject
{
	struct RHBObjectVtbl *lpVtbl;
	_IDL_SEQUENCE_octet object_key;
	RHBGIOPRequestStream *impl;
	RHBInterfaceDef *iface;
#ifdef SUPPORT_AUTO_RELEASE
	boolean auto_somRelease;
#endif
};

typedef void (*RHBRequest_Complete)(RHBRequest *request,void *refCon);

SOMEXTERN RHBRequest * SOMLINK RHBRequestNew(
			Environment *ev,
			RHBObject *object,
			SOMDObject SOMSTAR somobj,
			_IDL_SEQUENCE_string * ctx,
			Identifier operation,
			NVList SOMSTAR arg_list,
			NamedValue *result,
			Flags flags);

struct RHBRequest
{
	struct RHBRequestVtbl *lpVtbl;
	rhbatomic_t lUsage;
	struct
	{
		long send_flags;
		_IDL_SEQUENCE_octet object_key;
		SOMDObject SOMSTAR somobj;
		_IDL_SEQUENCE_string context_values;
		boolean send_context;
		Identifier operation;	
		NVList SOMSTAR arg_list;
		NamedValue *result;
		Flags flags;
		unsigned long lSequence;
		RHBGIOPRequestStream *impl;
		Principal SOMSTAR principal;
	} request;

	RHBRequest *next;
	RHBRequest_Complete completionPtr;
	RHBRequestList *container;
	void *refCon;

	RHBORB *orb;
	/* this marshalls the paremeters into 'marshalled_data ' */

	_IDL_SEQUENCE_octet marshalled_data;

	/* this writes the marshalled_data to the implementation def */

	boolean pending,
			deleting,
			has_got_response,
			was_local_target,
			cancelled;

	struct
	{
		unsigned long _length;
		unsigned long _maximum;
		struct RHBRequestMap *_buffer;
	} in_params;

	struct
	{
		GIOP_ReplyStatusType_1_0 reply_status;
		SOMObject SOMSTAR stream;
	} reply;

	GIOP_Version protocol_version;

	struct
	{
		Environment ev;
#ifdef USE_THREADS
		struct RHBORB_thread_task task;
#endif
	} local;
	/* used for 'get_next_response'.
		where each thread may have a pool */
	struct RHBORB_request_pool_t *pool;
};

struct RHBRequestVtbl
{
	void (*AddRef)(struct RHBRequest *);
	void (*Release)(struct RHBRequest *);
	void (*cancel)(struct RHBRequest *);
	ORBStatus (*add_arg)(struct RHBRequest *,Environment *,Identifier,TypeCode,void *,long,unsigned long);
	ORBStatus (*invoke)(struct RHBRequest *somThis,Environment *ev,Flags flags,SOMDServer SOMSTAR somdServer);
	ORBStatus (*send)(struct RHBRequest *somThis,Environment *ev,Flags flags,boolean bInvoke,SOMDServer SOMSTAR somdServer);
	ORBStatus (*get_response)(struct RHBRequest *somThis,Environment *ev,Flags flags);
	void (*SetComplete)(struct RHBRequest *somThis,Environment *ev);
	void (*SetCompletionProc)(struct RHBRequest *somThis,RHBRequest_Complete pfn,void *refCon);
	void (*reply_received)(RHBRequest *,Environment *,GIOP_MessageHeader *,GIOP_ReplyHeader_1_0 *,SOMObject SOMSTAR stream);
/*	ORBStatus (*marshal_request)(RHBRequest *somThis,Environment *ev,CORBA_DataOutputStream SOMSTAR stream);*/
	void (*send_marshalled)(RHBRequest *somThis,Environment *ev,RHBGIOPRequestStream *def);
/*	void (*forward)(RHBRequest *somThis,Environment *ev,SOMDObject SOMSTAR newTarget);
	ORBStatus (*send_local)(RHBRequest *somThis,Environment *ev,Flags flags);*/
};

#define RHBRequest_AddRef(a)					(a)->lpVtbl->AddRef((a))
#define RHBRequest_Release(a)					(a)->lpVtbl->Release((a))
#define RHBRequest_cancel(a)					(a)->lpVtbl->cancel((a))
#define RHBRequest_add_arg(a,b,c,d,e,f,g)		(a)->lpVtbl->add_arg((a),b,c,d,e,f,g)
#define RHBRequest_invoke(a,b,c,d)				(a)->lpVtbl->invoke((a),b,c,d)
#define RHBRequest_send(a,b,c,d,e)				(a)->lpVtbl->send((a),b,c,d,e)
#define RHBRequest_get_response(a,b,c)			(a)->lpVtbl->get_response((a),b,c)
#define RHBRequest_SetComplete(a,b)				(a)->lpVtbl->SetComplete((a),b)
#define RHBRequest_SetCompletionProc(a,b,c)		(a)->lpVtbl->SetCompletionProc((a),b,c)
#define RHBRequest_send_marshalled(a,b,c)		(a)->lpVtbl->send_marshalled((a),b,c)
#define RHBRequest_reply_received(a,b,c,d,e)	(a)->lpVtbl->reply_received((a),b,c,d,e)

#ifdef _DEBUG
void RHBServerRequest_dump_chain(RHBServerRequest *somThis);
#endif

RHBServerRequest * RHBServerRequestNew(
			Environment *ev,
			RHBGIOPRequestStream *impl,
			GIOP_MessageHeader *message,
			GIOP_RequestHeader_1_0 *header,
			SOMObject SOMSTAR stream);

RHBLocateRequest * RHBLocateRequestNew(
		Environment *ev,
		RHBGIOPRequestStream *impl,
		GIOP_MessageHeader *message,
		GIOP_LocateRequestHeader *hdr,
		SOMObject SOMSTAR in);
																			
#ifdef USE_SELECT
typedef void (SOMLINK * RHBServerRequest_Process)(
					RHBServerRequest *somThis,
					Environment *ev);
void RHBServerRequest_add_to_impl_queue(
			RHBServerRequest *somThis,
			Environment *ev,
			RHBServerRequest_Process pfn);
#endif

struct RHBServerRequest
{
	struct RHBServerRequestVtbl *lpVtbl;
	rhbatomic_t lUsage;
	RHBORB *orb;
	boolean closing;
	boolean cancelled;
	boolean finished;
	RHBGIOPRequestStream *impl;			/* request came in on this */
	Principal SOMSTAR principal;
	SOMDServer SOMSTAR somdServer;
	RHBORB_ThreadTask *executing_thread;

	struct
	{
		RHBServerRequest **execute_chain;
		char *state_text;
		char *state_file;
		int state_line;
	} metrics;

#define RHBServerRequest_state(a,b)   { a->metrics.state_text=b; a->metrics.state_file=__FILE__; a->metrics.state_line=__LINE__; }

#ifdef USE_THREADS
	struct RHBORB_thread_task task;
#else
	struct
	{
		RHBServerRequest_Process process_request;
		struct RHBServerRequest *next;
	} qel;
	void (SOMLINK * exec_st)(RHBServerRequest *somThis,Environment *ev);
#endif

	GIOP_Version protocol_version;
	GIOP_RequestHeader_1_0 header;
	SOMObject SOMSTAR in_stream;
	NVList SOMSTAR *param_seq;

	RHBServerRequest *next;

	/* this is a cache for the current request
		data is reference to data in context list in header */

	IIOP_SOMDServiceContextList somd_contexts;
};

struct RHBServerRequestVtbl
{
	void (*AddRef)(struct RHBServerRequest *);
	void (*Release)(struct RHBServerRequest *);
	void (*cancel)(struct RHBServerRequest *,Environment *ev);
	void (*push_current)(struct RHBServerRequest *,struct RHBServerRequest **);
	void (*pop_current)(struct RHBServerRequest *,struct RHBServerRequest **);
	ORBStatus (*set_result)(struct RHBServerRequest *,Environment *,any *);
	ORBStatus (*params)(struct RHBServerRequest *,Environment *,NVList SOMSTAR *);
	Context SOMSTAR (*ctx)(struct RHBServerRequest *,Environment *);
	Principal SOMSTAR (*get_principal)(struct RHBServerRequest *,Environment *);
};

#define RHBServerRequest_AddRef(x)  x->lpVtbl->AddRef(x)
#define RHBServerRequest_Release(x) x->lpVtbl->Release(x)
#define RHBServerRequest_cancel(x,e)  x->lpVtbl->cancel(x,e)
#define RHBServerRequest_push_current(x,e)  x->lpVtbl->push_current(x,e)
#define RHBServerRequest_pop_current(x,e)  x->lpVtbl->pop_current(x,e)
#define RHBServerRequest_set_result(x,a,b)	x->lpVtbl->set_result(x,a,b)
#define RHBServerRequest_params(x,a,b)		x->lpVtbl->params(x,a,b)
#define RHBServerRequest_ctx(x,a)			x->lpVtbl->ctx(x,a)
#define RHBServerRequest_get_principal(x,a)	x->lpVtbl->get_principal(x,a)

/* development bits */

#define RHBGIOPORB_SOM_debug(x,y,z,a)

#ifndef RHBGIOPORB_SOM_debug
SOMEXTERN void RHBGIOPORB_SOM_debug(char * x,char * y,char * file,int line);
#endif

#endif /* __RHBGIOP_H__ */

#ifndef SOMRealloc
	#define SOMRealloc   SOMRealloc_is_banned
	#define SOMCalloc	 SOMCalloc_is_banned
#endif

extern char *somd_dupl_string(const char *p);

#if defined(_WIN32) && defined(WIN32_LEAN_AND_MEAN)
	#ifndef UUID_DEFINED
		typedef struct tagUUID
		{
			char data[16];
		} UUID;
	#endif
#else
	typedef struct tagUUID
	{
		char data[16];
	} UUID;
#endif

/* this is a cheat, as SOMRefObject does not exist on Mac */

#ifndef _SOMRefObject
#define _SOMRefObject         SOMRefObjectClassData.classObject
#endif

extern RHBProtocolRef SOMLINK RHBORB_create_protocol(RHBORB *orb);

#ifdef RHBDebug_objects
	typedef struct RHBDebugBlock
	{
		char *type;
		long count;
	} RHBDebugBlock;

	#define RHBDebugBlockDef(x)		  extern RHBDebugBlock RHBDebugBlock_##x;
	#define RHBDebugBlockDefData(x)   RHBDebugBlock RHBDebugBlock_##x={#x,0};
	void RHBDebugCreated(RHBDebugBlock *,void *);
	void RHBDebugDeleted(RHBDebugBlock *,void *);
	void RHBDebugReport(void);
	void RHBDebugReportBlock(RHBDebugBlock *);
	#define RHBDebug_created(x,y)     RHBDebugCreated(&RHBDebugBlock_##x,y);
	#define RHBDebug_deleted(x,y)     RHBDebugDeleted(&RHBDebugBlock_##x,y);

	RHBDebugBlockDef(RHBServerRequest)
	RHBDebugBlockDef(RHBRequest)
	RHBDebugBlockDef(RHBObject)
	RHBDebugBlockDef(RHBGIOPRequestStream)
	RHBDebugBlockDef(RHBORB)
	RHBDebugBlockDef(RHBContained)
	RHBDebugBlockDef(RHBInterfaceDef)
#else
	#define RHBDebug_created(x,y)
	#define RHBDebug_deleted(x,y)
	#define RHBDebugBlockDefData(x)
	#define RHBDebugReport()
#endif

#ifdef _DEBUG
	#ifdef _WIN32
		#ifdef USE_THREADS
			#ifndef RHBDebug_yield
	/*			#define RHBDebug_yield     Sleep(50);*/
			#endif
		#endif
	#endif
#endif

#ifndef RHBDebug_yield
	#define RHBDebug_yield
#endif

#ifdef _PLATFORM_MACINTOSH_
RHBProtocolRef RHBSOMD_load_rhbnetot_protocol(void);
#endif

/* USE_ONESHOTIO is used because in some cases
	having to wait for an EWOULDBLOCK is not
	practical (for example 'provens MIT pthreads' */

#ifndef _PLATFORM_MACINTOSH_
/*	#ifdef USE_SELECT
		#define USE_ONESHOTIO
	#else
		#ifdef USE_PTHREADS
			#define USE_ONESHOTIO
		#endif
	#endif*/
	/* apart from the Mac. we're using select or poll to provide
		notifications of when ready to read/write */
	#define USE_ONESHOTIO
#endif


#ifdef _WIN32
	#define IS_UNUSABLE_SOCKET(fd)     (fd==INVALID_SOCKET)
#else
	#define MAX_FD_NUM		((sizeof(fd_set)<<3)-1)
	#define IS_UNUSABLE_SOCKET(fd)  ((fd>=MAX_FD_NUM))
#endif

/* end of RHBGIOP.H */


