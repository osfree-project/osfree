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

#define BUILD_RHBSOMD_C
#define SUSPICIOUS_USAGE_COUNT   (1 << ((sizeof(long) <<3)-2))

#ifndef HAVE_CONFIG_H
#	error HAVE_CONFIG_H
#endif

#ifndef BUILD_SOMD
	#error BUILD_SOMD
#endif

#include <rhbopt.h>

#include <rhbsomd.h>
#include <rhbgiops.h>
#include <somuutil.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif

SOMInitModule_begin(somd)

	RHBOPT_unused(classname)
	RHBOPT_unused(minor)
	RHBOPT_unused(major)

	SOMInitModule_new(NVList);
	SOMInitModule_new(Request);
	SOMInitModule_new(Context);
	SOMInitModule_new(Principal);
	SOMInitModule_new(ImplRepository);
	SOMInitModule_new(ORB);
	SOMInitModule_new(BOA);
	SOMInitModule_new(ImplementationDef);
	SOMInitModule_new(SOMOA);
	SOMInitModule_new(SOMDObject);
	SOMInitModule_new(SOMDServer);
	SOMInitModule_new(SOMDMetaproxy);
	SOMInitModule_new(SOMDClientProxy);
	SOMInitModule_new(SOMDServerMgr);
	SOMInitModule_new(ObjectMgr);
	SOMInitModule_new(SOMDObjectMgr);
	SOMInitModule_new(RHBORBStream_GIOPInput);
	SOMInitModule_new(RHBORBStream_GIOPOutput);
	SOMInitModule_new(IOP_Codec);
	SOMInitModule_new(IOP_CodecFactory);
	SOMInitModule_new(PortableServer_DynamicImpl);
	SOMInitModule_new(ServerRequest);

#ifndef USE_APPLE_SOM
	SOMInitModule_new(FENC_FactoryENC);
#endif

#ifdef DynamicTest_MajorVersion
	SOMInitModule_new(DynamicTest);
#endif

SOMInitModule_end

#ifdef _PLATFORM_MACINTOSH_XXX
static void * my_SOMCalloc(size_t s,size_t n);
static void * my_SOMMalloc(size_t s);
static void * (*old_SOMMalloc)(size_t);
static void * (*old_SOMCalloc)(size_t,size_t);

static void * my_SOMMalloc(size_t s)
{
	Str255 str;
	void *pv=old_SOMMalloc(s);
	if (pv) return pv;
	str[0]=sprintf((char *)&str[1],"SOMMalloc(%d) failed",s);
	DebugStr(str);
	return pv;
}

static void * my_SOMCalloc(size_t s,size_t n)
{
	Str255 str;
	void *pv=old_SOMCalloc(s,n);
	if (pv) return pv;
	str[0]=sprintf((char *)&str[1],"SOMCalloc(%d,%d) failed",s,n);
	DebugStr(str);
	return pv;
}
#endif

#ifdef RHBOPT_SHARED_DATA
static RHBGIOPORB_Globals *somd_GlobalsNew(void);
#endif

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_ATFORK
			static void somd_child(void)
			{
				if (SOMD_ORBObject)
				{
					Environment ev={NO_EXCEPTION,{NULL,NULL},NULL};
					RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,&ev);
					if (orb)
					{
						if (orb->protocol)
						{
							RHBProtocol_atfork_child(orb->protocol);
						}
					}
				}
			}
		#endif
	#endif
#endif

SOMEXTERN void SOMLINK SOMD_Init(Environment *ev)
{
	long i;
	SOMD_Globals *globals=
#ifdef RHBOPT_SHARED_DATA
		somd_GlobalsNew();
#else
		&somd_globals;
#endif
	RHBORB_guard(0)
	somd_atomic_inc(&globals->Init_count);
	RHBORB_unguard(0)
	i=globals->Init_count;

	if (i==1)
	{
#ifdef _PLATFORM_MACINTOSH_
	#ifdef SOMCalloc
		#undef SOMCalloc
	#endif
#endif

		RHBORB *c_orb=NULL;
		ORB SOMSTAR orb=
#ifdef somNewObject
		somNewObject(ORB);
#else
		ORBNew();
#endif
		if (orb)
		{
			c_orb=ORB__get_c_orb(orb,ev);
		}

		if (c_orb)
		{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_ATFORK
			pthread_atfork(NULL,NULL,somd_child);
		#endif
	#else
	#endif
#endif
		}
		else
		{
			{
				Environment ev2;
				SOM_InitEnvironment(&ev2);
				SOMD_Uninit(&ev2);
				SOM_UninitEnvironment(&ev2);
			}

			RHBOPT_throw_StExcep(ev,NO_RESOURCES,NoSOMDInit,NO);
		}
	}
}

SOMEXTERN void SOMLINK SOMD_Uninit(Environment *ev)
{
	SOMD_Globals *globals=&somd_globals;
	RHBOPT_ASSERT(ev)
	RHBOPT_ASSERT(!ev->_major)

	if (ev && globals)
	{
		if (!somd_atomic_dec(&globals->Init_count))
		{
			if (ev->_major)
			{
				somdExceptionFree(ev);
			}

			ORB_shutdown(SOMD_ORBObject,ev,0);
			ORB_somRelease(SOMD_ORBObject);
		}
	}

	somutresetenv(NULL);
}

#ifdef USE_THREADS
static void RHBSOMD_thread_ended(void *pv)
{
#ifdef _DEBUG
	RHBORB_ThreadTask *thread=pv;

	RHBOPT_ASSERT(!thread)
#endif
}
#endif

static void somd_globals_free(SOMD_Globals *globals,Environment *ev)
{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
			pthread_cond_destroy(&globals->p_event);
		#ifdef HAVE_PTHREAD_KEY_DELETE
			pthread_key_delete(globals->p_thread_key);
		#endif
	#else
			CloseHandle(globals->h_event);
			TlsFree(globals->thread_key);
	#endif
#endif
#ifdef RHBOPT_SHARED_DATA
	globals->lpVtbl->Destroy(globals);
#endif
}

static void somd_globals_Init(SOMD_Globals *globals,Environment *ev)
{
	if (!globals->initialised)
	{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
			pthread_cond_init(&globals->p_event,RHBOPT_pthread_condattr_default);
		#ifdef HAVE_PTHREAD_KEY_CREATE
			pthread_key_create(&globals->p_thread_key,RHBSOMD_thread_ended);
		#else
			pthread_keycreate(&globals->p_thread_key,RHBSOMD_thread_ended);
		#endif
	#else
			globals->h_event=CreateEvent(0,0,0,0);
			globals->thread_key=TlsAlloc();
	#endif
#endif

		globals->initialised=1;
	}

	RHBSOM_Trace("calling SOMD_SOMInitModule")

	SOMInitModule_call(somd,ORB_MajorVersion,ORB_MinorVersion,"ORB");

	if ((!SOMD_ORBObject) && (ev->_major==NO_EXCEPTION))
	{
		RHBSOM_Trace("calling SOMD_ORBObject=somNewObject(ORB);")

#ifdef somNewObject
		SOMD_ORBObject=somNewObject(ORB);
#else
		SOMD_ORBObject=ORBNew();
#endif
	}

	RHBSOM_Trace("creating dynproxy for Repository")

	somdCreateDynProxyClass(ev,
			"Repository",
			Repository_MajorVersion,
			Repository_MinorVersion);

	if (!SOMD_ImplRepObject)
	{
#ifdef somNewObject
		SOMD_ImplRepObject=somNewObject(ImplRepository);
#else
		SOMD_ImplRepObject=ImplRepositoryNew();
#endif
	}

	if ((!SOMD_ObjectMgr) && (ev->_major==NO_EXCEPTION))
	{
		RHBSOM_Trace("somNewObject(SOMDObjectMgr)")

#ifdef somNewObject
		SOMD_ObjectMgr=somNewObject(SOMDObjectMgr);
#else
		SOMD_ObjectMgr=SOMDObjectMgrNew();
#endif
	}

	RHBSOM_Trace("SOMD_Init completed")
}

static void somd_globals_Uninit(SOMD_Globals *globals,Environment *ev)
{
	RHBSOM_Trace("releasing globals")

	if (globals->initialised)
	{
		RHBORB_guard(globals->orb)

		while (globals->orb)
		{
			RHBORB *orb=globals->orb;

			dump_somPrintf(("Waiting for ORB(%p) to disappear\n",orb));

#ifdef USE_THREADS
	#if defined(USE_PTHREADS) 
			somd_wait(&somd_globals.p_event,__FILE__,__LINE__);
	#else
			somd_wait(somd_globals.h_event,__FILE__,__LINE__);
	#endif
#else
			RHBORB_AddRef(orb);
			RHBORB_idle(orb,ev,0);
			RHBORB_Release(orb);
#endif
#ifdef _DEBUG
			if (globals->orb)
			{
				/* still waiting ! */
				RHBOPT_ASSERT(!globals->orb);
				break;
			}
#endif
		}

		RHBORB_unguard(globals->orb)

		RHBDebugReport();

		RHBSOM_Trace("SOMD_Uninit finished")

#ifdef _DEBUG
		if (SOMD_SOMOAObject)
		{
			debug_somPrintf(("SOMD_SOMOAObject=%p left over\n",SOMD_SOMOAObject));
			SOMD_bomb("global SOMD_SOMOAObject left over");
		}
		if (SOMD_ORBObject)
		{
			debug_somPrintf(("SOMD_ORBObject=%p left over\n",SOMD_ORBObject));
			SOMD_bomb("global SOMD_ORBObject left over");
		}
		if (SOMD_ImplRepObject)
		{
			debug_somPrintf(("SOMD_ImplRepObject=%p left over\n",SOMD_ImplRepObject));
			SOMD_bomb("global SOMD_ImplRepObject left over");
		}
		if (SOMD_ObjectMgr)
		{
			debug_somPrintf(("SOMD_ObjectMgr=%p left over\n",SOMD_ObjectMgr));
			SOMD_bomb("global SOMD_ObjectMgr left over");
		}
#endif

		globals->initialised=0;

		somd_globals_free(globals,ev);
	}
}

SOMEXTERN void SOMLINK SOMD_RegisterCallback(SOMObject SOMSTAR emanObj, SOMD_Callback *func)
{
	if (SOMD_ORBObject)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		ORB_register_callback(
			SOMD_ORBObject,
			&ev,
			emanObj,
			(somMethodPtr)func);
		SOM_UninitEnvironment(&ev);
	}
}


SOMEXTERN void SOMLINK SOMInitsomd(void)
{
	
}

SOMEXTERN void SOMLINK SOMD_NoORBfree(void)
{
}

SOMEXTERN unsigned char SOMLINK SOMD_QueryORBfree(void)
{
	return 0;
}


SOMEXTERN void SOMLINK SOMD_FlushInterfaceCache(
			Environment *ev,
			corbastring name)
{
	RHBOPT_unused(ev)
	RHBOPT_unused(name)
	/* name of item  to purge, it NULL then purge all */
}

SOMEXTERN SOMClass SOMSTAR SOMLINK somdCreateDynProxyClass(
			Environment *ev,
			corbastring targetClassName, 
			long classMajorVersion,
			long classMinorVersion)
{
	SOMClassMgr SOMSTAR mgr=somGetClassManagerReference();
	SOMClass SOMSTAR cls=SOMClassMgr_somFindClass(mgr,&targetClassName,classMajorVersion,classMinorVersion);
	SOMClass SOMSTAR __result=NULL;

	if (cls)
	{
		RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

		if (orb)
		{
			__result=RHBORB_create_proxy_class(
					orb,
					ev,
					mgr,
					orb->somClass_SOMDClientProxy_ref,
					cls);
		}

		somReleaseClassReference(cls);
	}

	somReleaseClassManagerReference(mgr);

	return __result;
}

SOMEXTERN void SOMLINK ORBfree(void *pv)
{
	RHBOPT_unused(pv)
}

#if defined(_PLATFORM_MACINTOSH_) && GENERATINGCFM
void SOMD_CFM_term(void);

RHBOPT_CFM_init(SOMD,initBlockPtr)
{
	if (initBlockPtr)
	{
		return 0;
	}

	return -1;
}

typedef RHBProtocolRef (*rhbnetot_loader)(void);
static struct
{
	CFragConnectionID connectionId;
	rhbnetot_loader loader;
} rhbnetot;

void SOMD_CFM_term(void)
{
	if (rhbnetot.loader)
	{
		CloseConnection(&rhbnetot.connectionId);
		rhbnetot.connectionId=0;
		rhbnetot.loader=0;
	}
}

RHBProtocolRef RHBSOMD_load_rhbnetot_protocol(void)
{
	Str255 err;
	Ptr ptr;
	OSErr e;
	CFragSymbolClass symClass;

	if (rhbnetot.loader)
	{
		return rhbnetot.loader();
	}
/*
#ifdef __CFM68K__
	somPrintf("loading shlb 'RHBNETOT'(68K)\n");
#else
	somPrintf("loading shlb 'RHBNETOT'(PPC)\n");
#endif
*/
	e=GetSharedLibrary("\pRHBNETOT",
#if 1
		kCompiledCFragArch,
#else
	#ifdef __CFM68K__
		kMotorola68KCFragArch,
	#else
		kPowerPCCFragArch,
	#endif
#endif
		kLoadCFrag,
		&rhbnetot.connectionId,
		&ptr,
		err);

	if (e)
	{
/*		err[1+err[0]]=0;
		somPrintf("load failed with %d, %s\n",(int)e,&err[1]);*/
		return 0;
	}

	e=FindSymbol(rhbnetot.connectionId,
			"\prhbnetot_protocol",
			&ptr,
			&symClass);

	if (e)
	{
		debug_somPrintf(("FindSymbol failed with %d\n",(int)e));
		CloseConnection(&rhbnetot.connectionId);
		rhbnetot.connectionId=0;
		return 0;
	}

	rhbnetot.loader=(rhbnetot_loader)ptr;

/*	somPrintf("loaded 'RHBNETOT'\n");*/

	if (rhbnetot.loader)
	{
		return rhbnetot.loader();
	}

	return 0;
}
#endif


#if defined(HAVE_DLL_INIT) || defined(_WIN32) || defined(__ELF__)

/* 
	Solaris ELF 'ld'
	-z initfirst

	Darwin Mach-O, 'gcc' uses 'libtool' which uses 'ld'

	-init sym

	NetBSD GNU ELF 'ld'

	-fini sym	(default _fini)
	-init sym	(default _init)

  */

void somd_init(void)
{
	/* this to be called when module is loaded
		to capture the migrated methods
		prior to SOMD_Init */

#define migrate_SOMDObject(x)  SOMDObjectClassData.x=SOMObjectClassData.x

	migrate_SOMDObject(get_implementation);
	migrate_SOMDObject(get_interface);
	migrate_SOMDObject(release);
	migrate_SOMDObject(duplicate);
	migrate_SOMDObject(create_request);
	migrate_SOMDObject(create_request_args);
	migrate_SOMDObject(is_proxy);

#undef migrate_SOMDObject
}

#endif

#ifdef _PLATFORM_WIN32_
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
	RHBOPT_unused(extra)
	RHBOPT_unused(hInst)

 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
				somd_init();

				if (!somd_DllMain(hInst,reason,extra)) return 0;

           		return 1;
	        case DLL_PROCESS_DETACH:
            	return somd_DllMain(hInst,reason,extra);;
			case DLL_THREAD_DETACH:
#ifdef USE_THREADS
	#ifndef USE_PTHREADS
				RHBSOMD_thread_ended((void *)TlsGetValue(somd_globals.thread_key));
	#endif
#endif
				return somd_DllMain(hInst,reason,extra);
	}

   	return 1;
}
#endif

/* this is a SOM 3.0 style API */

ImplId SOMLINK somdExtractUUID(Environment * ev,
                                       ReferenceData * id)
{
	if (id->_length >= OBJECTKEY_PREFIX_LEN)
	{
		if (id->_buffer[0]==3)
		{
			unsigned int i=OBJECTKEY_PREFIX_LEN-2;
			char *p=SOMMalloc(i+1);

			if (p)
			{
				memcpy(p,id->_buffer+1,i);
				p[i]=0;
				return p;
			}
		}
	}

	return NULL;
}

ReferenceData SOMLINK somdGetDefaultObjectKey(
                                       Environment * ev,
                                       corbastring ImplId)
{
	ReferenceData seq={0,0,NULL};
/*	char buf[256];*/

	/* in true DSOM, this is the character code 0x03 followed
		by string version of impl_id and trailing null character,
		always a length of 38(decimal) characters */

	RHBOPT_unused(ev)

	RHBOPT_ASSERT(ev && (ev->_major==NO_EXCEPTION));

/*	seq._length=sprintf(buf,"SOM|2|%s|SOMDServer",ImplId);
	seq._maximum=seq._length;
	seq._buffer=SOMMalloc(seq._length);
	memcpy(seq._buffer,buf,seq._length);
*/
	if (!ImplId)
	{
		/* official IBM DSOM bombs heavily if you pass a NULL pointer
			into this routine, in our case it will create a object
			key for the 'somdd' dameon which always has the same impl_id
			on each machine 
		*/
		RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

		ImplId=RHBORB_get_somdd_implid(orb);
	}

	seq._length=(long)(2+strlen(ImplId));
	seq._maximum=seq._length;
	seq._buffer=SOMMalloc(seq._length);
	seq._buffer[0]=3;
	memcpy(&seq._buffer[1],ImplId,seq._length-1);

	return seq;
}

int SOMLINK somdStartProcess(Environment *ev,
			       char *path,
			       char *name,
			       char *implid,
			       unsigned long winSvrObj, corbastring title)
{
	RHBOPT_unused(ev)
	RHBOPT_unused(path)
	RHBOPT_unused(name)
	RHBOPT_unused(implid)
	RHBOPT_unused(winSvrObj)
	RHBOPT_unused(title)

	return 0;
}

/* this waits until the 'somdd' is ready */

boolean SOMLINK somdDaemonReady(Environment *ev,
                                       long timeout)
{
	RHBOPT_unused(ev)
	RHBOPT_unused(timeout)
	return 0;
}

boolean SOMLINK somdDaemonRequired(Environment *ev)
{
	RHBOPT_unused(ev)
	return 0;
}

struct somdCreate_data
{
	SOMDServer SOMSTAR server;
	Request SOMSTAR request;
};

RHBOPT_cleanup_begin(somdCreate_cleanup,pv)

struct somdCreate_data *data=pv;

	if (data->server) somReleaseObjectReference(data->server);
	if (data->request) somReleaseObjectReference(data->request);

RHBOPT_cleanup_end

SOMEXTERN SOMObject SOMSTAR SOMLINK somdCreate(Environment *ev,char *clsName,boolean doInit)
{
	SOMObject SOMSTAR object=NULL;

	RHBOPT_unused(doInit)

	if (!ev->_major)
	{
		if ((!SOMD_ORBObject) || (!SOMD_ObjectMgr))
		{
			RHBOPT_throw_StExcep(ev,INV_OBJREF,UnexpectedNULL,MAYBE);
		}
		else
		{
			struct somdCreate_data data={NULL,NULL};

			RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

			RHBOPT_cleanup_push(somdCreate_cleanup,&data);

			data.server=SOMDObjectMgr_somdFindAnyServerByClass(SOMD_ObjectMgr,ev,
					clsName);

			if (!ev->_major)
			{
				if (data.server)
				{
					char *hints=NULL;

					if (RHBORB_isSOMDObject(orb,data.server))
					{
						NamedValue result=RHBSOMD_Init_NamedValue;

						result.argument._type=TC_Object;
						result.argument._value=&object;

						SOMDObject_create_request(data.server,ev,0,
								"somdCreateObj",
								0,
								&result,
								&data.request,
								0);

						if (!ev->_major)
						{
							Request_add_arg(data.request,ev,"objclass",
								TC_string,
								&clsName,
								0,ARG_IN);

							Request_add_arg(data.request,ev,"hints",
								TC_string,
								&hints,
								0,ARG_IN);

							Request_invoke(data.request,ev,0);
						}
					}
					else
					{
						object=SOMDServer_somdCreateObj(data.server,ev,clsName,hints);
					}	
				}
				else
				{
					RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,ImplNotFound,NO);
				}
			}

			RHBOPT_cleanup_pop();
		}

	}

	return object;
}


/* this should be thread-local data,
	so that multiple threads can have their
	own queue of requests,
	or at least process scoped
	for _WIN16 and ASLM */

static RHBORB_request_pool_t *RHBORB_get_request_pool(Environment *ev)
{
	RHBORB *c_orb=ORB__get_c_orb(SOMD_ORBObject,ev);

	if (c_orb) return &c_orb->pool;

	RHBOPT_throw_StExcep(ev,INTERNAL,NoSOMDInit,NO);

	return NULL;
}

static void RHBORB_request_pool_unhook(
		RHBRequest *req,
		void *pv)
{
	RHBORB_request_pool_t *pool=req->pool;

/*	somPrintf("request %p is finished\n",req);*/
	RHBORB_guard(0)

	RHBOPT_ASSERT(!req->container);

	RHBRequestList_add(&pool->completed,req);

	pool->outstanding--;

#ifdef USE_THREADS
	if (pool->waiting)
	{
#ifdef USE_PTHREADS
		pthread_cond_signal(pool->event);
#else
		if (!SetEvent(pool->event)) 
		{
#ifdef _M_IX86
			__asm int 3;
#endif
		}
#endif
	}
#endif
	RHBORB_unguard(0)
	RHBOPT_unused(pv)
}

SOMEXTERN ORBStatus SOMLINK send_multiple_requests (
			Request SOMSTAR	reqs[],	
			Environment	*ev,
			long		count,
			Flags		invoke_flags)
{
	RHBORB_request_pool_t *pool=RHBORB_get_request_pool(ev);
	ORBStatus status=0;

	if (ev->_major)
	{
		return SOMDERROR_BadEnvironment;
	}

	if (invoke_flags & INV_NO_RESPONSE)
	{
		int i=0;

		while (i < count)
		{
			status=Request_send(reqs[i],ev,invoke_flags);

			RHBOPT_ASSERT(!status);

			i++;
		}
	}
	else
	{
		int i=0;

		while (i < count)
		{
			RHBRequest *request;

			RHBORB_guard(0)

			/* add to global response list */

			request=Request__get_c_request(reqs[i],ev);
			request->pool=pool;

			RHBRequest_SetCompletionProc(
					request,
					RHBORB_request_pool_unhook,
					reqs[i]);
	
			pool->outstanding++;

			RHBORB_unguard(0)

			status=Request_send(reqs[i],ev,invoke_flags);
			
			RHBOPT_ASSERT(!status);

			i++;

			RHBRequest_Release(request);
		}
	}

	return status;
}

static Request SOMSTAR RHBORB_request_pop_completed(
			RHBORB_request_pool_t *pool,
			Environment *ev)
{
	RHBRequest *request=pool->completed.first;

	while (request)
	{
		RHBOPT_ASSERT(!request->pending);

		if (!request->pending)
		{
			Request SOMSTAR req=Request_somDuplicateReference(request->refCon);

			RHBRequestList_remove(&pool->completed,request);

			RHBOPT_ASSERT(SOMObject_somIsA(req,_Request))

			return req;
		}

		request=request->next;
	}

	RHBOPT_unused(ev)

	return NULL;
}

struct get_next_response
{
	RHBORB_request_pool_t *pool;
	Request SOMSTAR request;
	boolean locked,deleteEvents;
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_t p_event;
	#else
		HANDLE h_event;
	#endif
#endif
};

RHBOPT_cleanup_begin(get_next_response_cleanup,pv)

	struct get_next_response *data=pv;

	if (data->pool)
	{
		data->pool->waiting=0;
#ifdef USE_THREADS
		data->pool->event=NULL;
#endif
		data->pool=NULL;
	}

	if (data->deleteEvents)
	{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_destroy(&data->p_event);
	#else
		if (!CloseHandle(data->h_event)) 
		{
		#ifdef _M_IX86
			__asm int 3
		#endif
		}
	#endif
#endif
	}

	if (data->locked)
	{
		data->locked=0;
		RHBORB_unguard(0)
	}

	if (data->request)
	{
		somReleaseObjectReference(data->request);
		data->request=NULL;
	}

RHBOPT_cleanup_end

SOMEXTERN ORBStatus SOMLINK get_next_response (
			Environment	*ev,
			Flags		response_flags,
			Request SOMSTAR	*req)
{
	RHBOPT_volatile ORBStatus st=SOMDERROR_NoMessages;
	struct get_next_response data={NULL,NULL,0,0};

	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);
	RHBOPT_ASSERT(req);
	RHBOPT_ASSERT(!req[0]);

	if (!ev) 
	{
		return SOMDERROR_BadEnvironment;
	}

	if (ev->_major)	return SOMDERROR_BadEnvironment;
	if (!req) return SOMDERROR_UnexpectedNULL;

	data.pool=RHBORB_get_request_pool(ev);

	RHBOPT_ASSERT(data.pool);

	if (!data.pool) return SOMDERROR_UnexpectedNULL;

	*req=NULL;

	RHBORB_guard(0)

	data.locked=1;

	RHBOPT_cleanup_push(get_next_response_cleanup,&data);

	if (data.pool->waiting)
	{
		data.pool=NULL;
		st=SOMDERROR_AccessDenied;
	}
	else
	{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_init(&data.p_event,RHBOPT_pthread_condattr_default);
		data.pool->event=&data.p_event;
	#else
		data.h_event=CreateEvent(NULL,0,0,NULL);
		data.pool->event=data.h_event;
	#endif
#endif
		data.deleteEvents=1;
		data.pool->waiting=1;

		if (response_flags & RESP_NO_WAIT)
		{
			data.request=RHBORB_request_pop_completed(data.pool,ev);
		}
		else
		{
	#ifndef USE_THREADS
			RHBORB *c_orb=ORB__get_c_orb(SOMD_ORBObject,ev);
	#endif
			do
			{
				data.request=RHBORB_request_pop_completed(data.pool,ev);

				if (!data.request)
				{
					if (ev->_major || !data.pool->outstanding)
					{
						break;
					}

	#ifdef USE_THREADS
					somd_wait(data.pool->event,__FILE__,__LINE__);
	#else
					RHBORB_idle(c_orb,ev,response_flags);
	#endif
				}

			} while (!data.request);
		}

		data.pool->waiting=0;
#ifdef USE_THREADS
		data.pool->event=NULL;
#endif
		data.pool=NULL;
	}

	data.locked=0;

	RHBORB_unguard(0)

	if (data.request)
	{
		st=Request_get_response(data.request,ev,response_flags);
		*req=data.request;
	}

	RHBOPT_cleanup_pop();

	return st;
}

#ifdef SOM_RESOLVE_DATA
	#ifdef RHBOPT_SHARED_DATA
		#define SOMD_EXPORT_DATA(t,n)  t * SOMLINK resolve_SOMD_##n(void) { \
					RHBGIOPORB_Globals *g=resolve_somd_globals(); \
					if (g) return &g->n; return NULL; }
	#else
		#define SOMD_EXPORT_DATA(t,n)  t * SOMLINK resolve_SOMD_##n(void) { return &SOMD_##n; }
	#endif

SOMD_EXPORT_DATA(ORB SOMSTAR,ORBObject)
SOMD_EXPORT_DATA(SOMDServer SOMSTAR,ServerObject)
SOMD_EXPORT_DATA(SOMDObjectMgr SOMSTAR,ObjectMgr)
SOMD_EXPORT_DATA(SOMOA SOMSTAR,SOMOAObject)
SOMD_EXPORT_DATA(ImplementationDef SOMSTAR,ImplDefObject)
SOMD_EXPORT_DATA(ImplRepository SOMSTAR,ImplRepObject)
SOMD_EXPORT_DATA(Context SOMSTAR,DefaultContext)
SOMD_EXPORT_DATA(long,TraceLevel)
SOMD_EXPORT_DATA(long,DebugFlag)
SOMD_EXPORT_DATA(long,RecvBufferSize)
#endif

#if defined( _WIN32) && !defined(HAVE_INET_PTON)
const char * __stdcall inet_ntop(int af, const void *src, char *dst, int size)
{
	switch (af)
	{
	case AF_INET:
		{
			const unsigned char *addr=src;
			char buf[16];
#ifdef HAVE_SNPRINTF
			int i=snprintf(buf,sizeof(buf),
#else
			int i=sprintf(buf,
#endif
					"%d.%d.%d.%d",
					addr[0],addr[1],addr[2],addr[3]);

			if ((i+1)<size)
			{
				memcpy(dst,buf,i+1);
				return dst;
			}
		}
		break;
#ifdef AF_INET6
	case AF_INET6:
		{
			const WORD *addr=src;

			snprintf
				(dst,size,
				"%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
				ntohs(addr[0]),
				ntohs(addr[1]),
				ntohs(addr[2]),
				ntohs(addr[3]),
				ntohs(addr[4]),
				ntohs(addr[5]),
				ntohs(addr[6]),
				ntohs(addr[7]));

			return dst;
		}
		break;
#endif
	}
	return NULL;
}

int __stdcall inet_pton(int af, const char *src, void *dst)
{
	/* this assignment is to check prototype at compile time */

	switch (af)
	{
	case AF_INET:
		{
			struct in_addr addr;
			addr.s_addr=inet_addr(src);

			if (addr.s_addr != INADDR_NONE)
			{
				*(struct in_addr *)dst=addr;

				return 1;
			}

			/* not parseable */

			return 0;
		}
		break;
#ifdef AF_INET6
	case AF_INET6:
#ifdef HAVE_GETADDRINFO
		{
			struct addrinfo hints, *res=NULL, *ressave=NULL;

			memset(&hints, 0, sizeof(hints));

			hints.ai_family = af;

			if (getaddrinfo(src, NULL, &hints, &res) != 0)
			{
				return -1;
			}

			ressave=res;

			while (res)
			{
				memcpy(dst,res->ai_addr,res->ai_addrlen);
				res=res->ai_next;
			}

			freeaddrinfo(ressave);

			return 0;
		}
#endif
		break;
#endif
	}

	/* cannot cope */

	return -1;
}
#endif

#ifdef RHBOPT_SHARED_DATA
static RHBGIOPORB_Globals *somd_globals_list;
static void somd_globals_Destroy(RHBGIOPORB_Globals *p)
{
	if (p)
	{
		if (p==somd_globals_list)
		{
			somd_globals_list=p->next;
		}
		else
		{
			RHBGIOPORB_Globals *q=somd_globals_list;
			while (q)
			{
				if (q->next == p)
				{
					q->next=p->next;
					break;
				}
				else
				{
					q=q->next;
				}
			}
		}

		SOMFree(p);
	}
}

static struct SOMD_GlobalsVtbl somd_globals_vtbl=
{
	somd_globals_Init,
	somd_globals_Uninit,
	somd_globals_Destroy
};

RHBGIOPORB_Globals *resolve_somd_globals(void)
{
	RHBGIOPORB_Globals *p=somd_globals_list;
	RHBGIOPORB_Globals *q=NULL;
#ifdef _PLATFORM_MACINTOSH_
	ProcessSerialNumber psn;
	GetCurrentProcess(&psn);
#else
	#ifdef _WIN32
		DWORD tid=GetCurrentThreadId();
	#else
		int pid=getpid();
	#endif
#endif

	while (p)
	{
#ifdef _PLATFORM_MACINTOSH_
		Boolean b=0;
		if (!SameProcess(&psn,&p->psn,&b) && b)
#else
	#ifdef _WIN32
		if (p->tid==tid) 
	#else
		if (p->pid==pid) 
	#endif
#endif
		{
			if (q)
			{
				/* move to front of list
					as this looks like the
					active process */

				q->next=p->next;
				p->next=somd_globals_list;
				somd_globals_list=p;
			}

			return p;
		}

		q=p;
		p=p->next;
	}

	return p;
}

static RHBGIOPORB_Globals *somd_GlobalsNew(void)
{
	RHBGIOPORB_Globals *p=resolve_somd_globals();

	if (!p)
	{
		p=SOMMalloc(sizeof(*p));
		if (p)
		{
			memset(p,0,sizeof(*p));

			p->lpVtbl=&somd_globals_vtbl;
	#ifdef _PLATFORM_MACINTOSH_
			GetCurrentProcess(&p->psn);
	#else
		#ifdef _WIN32
			p->tid=GetCurrentThreadId();
		#else
			p->pid=getpid();
		#endif
	#endif
			p->next=somd_globals_list;
			somd_globals_list=p;
		}
	}

	return p;
}
#else
ImplementationDef	SOMSTAR  SOMDLINK SOMD_ImplDefObject;
ImplRepository		SOMSTAR  SOMDLINK SOMD_ImplRepObject;
ORB					SOMSTAR  SOMDLINK SOMD_ORBObject;
SOMDObjectMgr		SOMSTAR  SOMDLINK SOMD_ObjectMgr;
SOMDServer			SOMSTAR  SOMDLINK SOMD_ServerObject;
SOMOA				SOMSTAR  SOMDLINK SOMD_SOMOAObject;
#ifdef USE_THREADS
SOMObject
#else
Sockets
#endif
					SOMSTAR	 SOMDLINK SOMD_SocketObject;
Context				SOMSTAR  SOMDLINK SOMD_DefaultContext;
long						 SOMDLINK SOMD_TraceLevel; 
long						 SOMDLINK SOMD_DebugFlag;
long						 SOMDLINK SOMD_RecvBufferSize;
static struct SOMD_GlobalsVtbl somd_globals_vtbl=
{
	somd_globals_Init,
	somd_globals_Uninit
};
SOMD_Globals somd_globals={&somd_globals_vtbl};
#endif

#ifdef somExceptionFree
	#undef somExceptionFree
#endif

#ifdef SOM_UninitEnvironment
	#undef SOM_UninitEnvironment
#endif

SOMEXTERN void SOMLINK somdExceptionFree(Environment *evOld)
{
	if (evOld)
	{
		if (evOld->_major)
		{
			if (evOld->exception._params && 
				(evOld->_major == USER_EXCEPTION) &&
				SOMD_ORBObject)
			{
				RHBORB *orb;
				Environment ev;
				TypeCode t;
				char *id=somExceptionId(evOld);

				SOM_InitEnvironment(&ev);

				orb=ORB__get_c_orb(SOMD_ORBObject,&ev);

				t=RHBORB_get_exception_tc(orb,
						evOld->_major,
						id);

				if (t)
				{
					SOMD_FreeType(&ev,evOld->exception._params,t);
					TypeCode_free(t,&ev);
				}
				else
				{
					debug_somPrintf(("somdExceptionFree() failed to find exception %s, %s:%d\n",
							id,__FILE__,__LINE__));
				}

				if (ev._major)
				{
					somdExceptionFree(&ev);
				}

				if (evOld->exception._params)
				{
					SOMFree(evOld->exception._params);
					evOld->exception._params=NULL;
				}
			}

			somExceptionFree(evOld);
		}
	}
}
