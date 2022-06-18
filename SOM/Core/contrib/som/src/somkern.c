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

/* $Id$ */

#define SOMKERN_C

#include <rhbopt.h>

#ifdef _WIN32
	#include <windows.h>
#endif

#define RHBSOMKD_STATIC
#define NEED_SOMKERN_RESOLVE_INDEX_MI

#define SOMObject_VA_STUBS
#define SOMObject_Class_Source
#define SOMClass_Class_Source
#define SOMClassMgr_Class_Source

#define SOM_Module_somobj_Source
#define SOM_Module_somcls_Source
#define SOM_Module_somcm_Source

#include <stdio.h>
#include <rhbmtut.h>
#include <somkern.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif

/*	#include <errno.h>*/

#ifdef HAVE_SIGNAL_H
	#include <signal.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
	#include <Errors.h>
	#define HAVE_DLFCN_H
#else
	#if !defined(USE_THREADS) && !defined(HAVE_RAISE)
		static int raise(int x) { return kill(getpid(),x); }
	#endif
#endif

typedef RHBSOMUT_KeyDataSet SOMClassMgrList;
typedef struct SOMClassMgrData *SOMClassMgrDataPtr;

#include "somobj.ih"
#include "somcls.ih"
#include "somcm.ih"

#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#		define USE_RHBMUTEX_LOCK
			static struct rhbmutex_t som_global_mutex
#		ifdef RHBMUTEX_INIT_DATA 
			=RHBMUTEX_INIT_DATA
#		endif
			;
#	else
		static CRITICAL_SECTION som_global_mutex;
#	endif
#else
	#if defined(_PLATFORM_MACINTOSH_) || defined(RHBOPT_SHARED_DATA) || defined(_WIN32S)
	#else
		static unsigned int som_critical_count;
		static sigset_t som_critical_masks[64];
		#ifdef somStartCriticalSection
			static struct
			{
				const char *file;
				int line;
			} debugCritical[64];
		#endif
	#endif
#endif

/* internal routines that have now been made static */

#ifdef _WIN32
	#define SOMKERN_DLL_TERM
#endif

#if defined(USE_THREADS) || defined(_WIN32)
	#define SOMKERN_DLL_INIT
	#define SOMKERN_END_THREAD
#endif

#ifdef SOMKERN_DLL_INIT
static void SOM_dll_init(void);
#endif
#ifdef SOMKERN_DLL_TERM
static void SOM_dll_term(void);
#endif

#ifdef SOMKERN_END_THREAD
static void SOMKERN_end_thread(void *evv);
#endif

static void SOMClass_unregistered(SOMClass SOMSTAR somSelf,somMethodTabPtr mtabs);
static som_thread_globals_t *SOMKERN_get_thread_globals(char make);
static struct somParentClassInfo *SOMKERN_get_this_class(somMethodTabPtr p);
static som_globals_t som_globals;
static unsigned long SOMKERN_total_defined_methods(somStaticClassInfo *sci);
static somMToken SOMKERN_index_to_somMToken(somMethodTabPtr mtab,unsigned int i);
#ifndef SOMKERN_resolve
static somMethodPtr SOMKERN_resolve(somMethodTabPtr mtab,somMToken token);
#endif
static unsigned int SOMKERN_resolve_index_mi(somMethodTabPtr mtab,somMToken token);
static struct somParentClassInfo * SOMKERN_get_info_for(somMethodTabPtr mtab,somMethodTabPtr parent);
static void SOMKERN_UnbootStrap(void);
static int SOMKERN_any_mi(somMethodTabPtr);
static int SOMKERN_count_unique_classes(somMethodTabPtr *,unsigned int);
static int SOMKERN_count_unique_methods(somMethodTabPtr *,unsigned int);
static int SOMKERN_is_method_in(somMethodTabPtr *,unsigned int,somMToken);
static void SOMKERN_set_up_parents(somMethodTabPtr,somMethodTabPtr *,int num);

/*static somMToken SOMKERN_find_method_by_name(somMethodTabPtr mtab,char *p,unsigned int *index);*/

/*static void SOMKERN_post_process(somStaticClassInfo *sci,somMethodTabPtr mtab);*/
static void SOMKERN_clear_somMethodTabPtr(somMethodTabPtr mtab);
static void SOMKERN_clear_somStaticClassInfo(somStaticClassInfo *sci);
static struct somMethodTabStruct * SOMKERN_resolve_mtab(struct somMethodTabStruct *mtab);
static void somcm_teardown(SOMClassMgr SOMSTAR);
static void SOMClassList_create(SOMClassMgrData *somThis);
static somMethodTabPtr SOMClassList_find(SOMClassMgrData *somThis,Environment *ev,char *p);
static long SOMClassList_remove(SOMClassMgrData *somThis,Environment *ev,somMethodTabPtr mtab);
static void SOMClassList_add(SOMClassMgrData *somThis,Environment *ev,somMethodTabPtr mtab);
static void SOMClassList_destroy(SOMClassMgrData *somThis);
static void SOMKERN_AllocateInstanceSize(struct somClassInfoMethodTab *);


#ifdef SOM_METHOD_THUNKS
static somToken SOMMallocEx(size_t t)
{
	return VirtualAlloc(NULL,t,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
}
static void SOMFreeEx(somToken pv)
{
	if (pv)
	{
		if (!VirtualFree(pv,0,MEM_RELEASE))
		{
			__asm int 3
		}
	}
}
#else
#	define SOMMallocEx(x)		SOMMalloc(x)
#	define SOMFreeEx(x)		SOMFree(x)
#endif

/* end of recently made static routines */

SOM_SEQUENCE_TYPEDEF(somCClassDataStructurePtr);
static somCClassDataStructurePtr SOM_Bootstrap_ccds[3]={
		(somCClassDataStructurePtr)(void *)&SOMObjectCClassData,
		(somCClassDataStructurePtr)(void *)&SOMClassCClassData,
		(somCClassDataStructurePtr)(void *)&SOMClassMgrCClassData};
static _IDL_SEQUENCE_somCClassDataStructurePtr SOM_Bootstrap_ccds_seq={
		3,3,SOM_Bootstrap_ccds};

#ifndef WIN32_DLLEXPORT
	#define WIN32_DLLEXPORT     
#endif

int WIN32_DLLEXPORT SOMDLINK SOM_TraceLevel=1;
int WIN32_DLLEXPORT SOMDLINK SOM_WarnLevel=0;
int WIN32_DLLEXPORT SOMDLINK SOM_AssertLevel=0;
long WIN32_DLLEXPORT SOMDLINK SOM_MajorVersion=SOMObject_MajorVersion;
long WIN32_DLLEXPORT SOMDLINK SOM_MinorVersion=SOMObject_MinorVersion;
long WIN32_DLLEXPORT SOMDLINK SOM_MaxThreads;
long WIN32_DLLEXPORT SOMDLINK SOM_IdTableSize;
somToken WIN32_DLLEXPORT SOMDLINK SOM_IdTable;

#if defined(_WIN32) && !defined(_WIN32S)
static DWORD som_tidMain;
#endif

SOM_Scope int SOMLINK SOMKERN_outchar(char c)
{
	if (c)
	{
		char buf[1];
#ifdef _WIN32
#	ifdef STD_OUTPUT_HANDLE
		HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
#	else
		HANDLE h=INVALID_HANDLE_VALUE;
#	endif
#endif
		if (c==7) return 0;


		buf[0]=c;

#ifndef _PLATFORM_MACINTOSH_
#ifdef _WIN32
		/* write to std out */
		if (h!=INVALID_HANDLE_VALUE)
		{
			DWORD dw=1;
			if (WriteFile(h,buf,dw,&dw,NULL))
			{
				return dw;
			}
		}
#else
		/* write to stdout */
		return write(1,buf,1);
#endif
#endif
	}

	return 0;
}

SOM_Scope void SOMLINK SOMKERN_error(int e_num,const char * file,int line)
{
	somPrintf("SOMError #%d at %s:%d\n",e_num,file,line);
#ifdef _WIN32
	RaiseException(e_num,EXCEPTION_NONCONTINUABLE,0,NULL);
#else
	#ifdef _PLATFORM_MACINTOSH_
		#ifdef USE_ASLM
			RAISE(e_num);
		#else
			pthread_exit(PTHREAD_CANCELED);
		#endif
	#else
		kill(getpid(),SIGUSR1);
	#endif
#endif
}

#ifdef RHBOPT_SHARED_DATA
#else
	somTD_SOMOutCharRoutine * WIN32_DLLEXPORT SOMDLINK SOMOutCharRoutine=SOMKERN_outchar;
	somTD_SOMError   * WIN32_DLLEXPORT SOMDLINK SOMError=SOMKERN_error;
	#ifndef USE_THREADS
		#define local_SOMOutCharRoutine		SOMOutCharRoutine
	#endif
#endif

#define SOMKERN_outchar do not use beyond here

#ifdef SOM_RESOLVE_DATA
	#define EXPORT_GLOBAL_DATA(x,y)   SOMEXTERN x * SOMLINK resolve_##y(void) { return &y; }

	EXPORT_GLOBAL_DATA(somTD_SOMMalloc *,SOMMalloc)
	EXPORT_GLOBAL_DATA(somTD_SOMCalloc *,SOMCalloc)
	EXPORT_GLOBAL_DATA(somTD_SOMFree *,SOMFree)
	EXPORT_GLOBAL_DATA(somTD_SOMRealloc *,SOMRealloc)
	EXPORT_GLOBAL_DATA(somTD_SOMCreateMutexSem *,SOMCreateMutexSem)
	EXPORT_GLOBAL_DATA(somTD_SOMRequestMutexSem *,SOMRequestMutexSem)
	EXPORT_GLOBAL_DATA(somTD_SOMReleaseMutexSem *,SOMReleaseMutexSem)
	EXPORT_GLOBAL_DATA(somTD_SOMDestroyMutexSem *,SOMDestroyMutexSem)
	EXPORT_GLOBAL_DATA(SOMClassMgr SOMSTAR,SOMClassMgrObject)
	EXPORT_GLOBAL_DATA(somTD_SOMGetThreadId *,SOMGetThreadId)
	EXPORT_GLOBAL_DATA(somTD_SOMLoadModule *,SOMLoadModule)
	EXPORT_GLOBAL_DATA(somTD_SOMDeleteModule *,SOMDeleteModule)
	EXPORT_GLOBAL_DATA(somTD_SOMClassInitFuncName *,SOMClassInitFuncName)
	EXPORT_GLOBAL_DATA(int,SOM_TraceLevel)
	EXPORT_GLOBAL_DATA(int,SOM_WarnLevel)
	EXPORT_GLOBAL_DATA(int,SOM_AssertLevel)
	EXPORT_GLOBAL_DATA(long,SOM_MajorVersion)
	EXPORT_GLOBAL_DATA(long,SOM_MinorVersion)
	EXPORT_GLOBAL_DATA(long,SOM_MaxThreads)
	EXPORT_GLOBAL_DATA(somToken,SOM_IdTable)
	EXPORT_GLOBAL_DATA(long,SOM_IdTableSize)
	EXPORT_GLOBAL_DATA(somTD_SOMError *,SOMError)
#endif

somMethodPtr SOMLINK somResolve(SOMObject SOMSTAR obj,somMToken mdata)
{
	RHBOPT_ASSERT(somIsObj(obj))

#ifdef SOM_METHOD_THUNKS
	/* a thunk is a very small somMToken, just containing a call instruction
		followed by a pointer to the real somMToken, we need to be able
		to cater for both thunks and true mtokens */
	RHBOPT_ASSERT(SOMMTOKEN_DATA(mdata));
	mdata=SOMMTOKEN_DATA(mdata);
	RHBOPT_ASSERT(mdata==SOMMTOKEN_DATA(mdata));
#endif

#ifdef _DEBUG
	if (!somTestCls(obj,somGetClassFromMToken(mdata),__FILE__,__LINE__))
	{
		const char *p=*(mdata->defined.somId_methodDescriptor);
		somPrintf("somResolve(%p,%s,%s)\n",obj,obj->mtab->className,p);
		return NULL;
	}
#endif

	RHBOPT_ASSERT(somClassInfoFromMtab(obj->mtab)==obj->mtab->classInfo)

	return SOMKERN_resolve(somMethodTabFromObject(obj),mdata);
}

SOMEXTERN somToken SOMLINK somDataResolve(SOMObject SOMSTAR obj, somDToken dataId)
{
	RHBOPT_ASSERT(dataId->cls->classObject);
	RHBOPT_ASSERT(somTestCls(obj,dataId->cls->classObject,__FILE__,__LINE__));
	RHBOPT_ASSERT(obj);
	RHBOPT_ASSERT(dataId->data_offset);

	if (somClassInfoFromMtab(somMethodTabFromObject(obj))->multiple_inheritance)
	{
		struct somParentClassInfo *q=somClassInfoFromMtab(somMethodTabFromObject(obj))->classes._buffer;

		while (q->cls != dataId->cls)
		{
			q++;
		}

		/* check for a non-inherited interface */
		RHBOPT_ASSERT(q->inherit_var) 

		return ((char *)obj)+q->data_offset;
	}

	return ((char *)obj)+dataId->data_offset;
}

void SOMLINK somEnvironmentEnd(void)
{
#ifdef RHBOPT_SHARED_DATA
	if (som_globals.apps)
	{
		struct som_thread_globals_t *ev=som_globals.apps;
#ifdef _WIN32
		DWORD tid=GetCurrentThreadId();
#else
		ProcessSerialNumber psn;
		if (GetCurrentProcess(&psn)) 
		{
			somPrintf("GetCurrentProcess failed\n");

			return;
		}
#endif

		somPrintf("somEnvironmentEnd - unhooking\n");

		while (ev)
		{
			while (ev)
			{
#ifdef _WIN32
				if (ev->tid==tid) break;
#else
				Boolean b=0;

				if (!SameProcess(&psn,&ev->psn,&b))
				{
					if (b) break;
				}
#endif

				ev=ev->next;
			}

			if (ev) 
			{
				somExceptionFree(&ev->ev);

				if (ev->repository)
				{
					SOMObject SOMSTAR rep=ev->repository;
					ev->repository=NULL;
					SOMObject_somFree(rep);
				}

				if ((ev==som_globals.apps) && !ev->next)
				{
					somPrintf("tearing down class mgr\n");

					somcm_teardown(SOMClassMgrObject);
				}

				if (ev==som_globals.apps)
				{
					som_globals.apps=ev->next;
				}
				else
				{
					struct som_thread_globals_t *p=som_globals.apps;

					while (p)
					{
						if (p->next==ev)
						{
							p->next=ev->next;
							break;
						}
						else
						{
							p=p->next;
						}
					}
				}

				SOMFree(ev);

				ev=som_globals.apps;

				if (!ev)
				{
					somPrintf("finally...\n");

#ifdef _PLATFORM_MACINTOSH_
					somaslm_unloadall();
#endif
				}
			}
		}
	}
#else
	if (som_globals.repository)
	{
		SOMObject SOMSTAR o=som_globals.repository;
		som_globals.repository=NULL;
#ifdef SOMObject_somDestruct
		SOMObject_somDestruct(o,1,0);
#else
		SOMObject_somFree(o);
#endif
	}
#endif

	if (SOMClassMgrObject
#ifdef RHBOPT_SHARED_DATA
		&& !som_globals.apps
#endif
		) 
	{
		somcm_teardown(SOMClassMgrObject);
		SOMClassMgr_somFree(SOMClassMgrObject);
	}

	SOMKERN_UnbootStrap();
}

struct somBuildClass
{
	boolean locked,failed,was_created;
	SOMClass SOMSTAR explicit_meta_class;
	SOMClass SOMSTAR meta_class;
	SOMClass SOMSTAR classObject;
	SOMClass_SOMClassSequence seq;
	SOMClass SOMSTAR parents[sizeof(long)<<3];
};

RHBOPT_cleanup_begin(somBuildClass_cleanup,pv)

struct somBuildClass *data=pv;

	if (data->locked) somEndCriticalSection();
#ifdef SOMClass_somRelease
	if (data->meta_class)
	{
		SOMClass_somRelease(data->meta_class);
	}
	if (data->explicit_meta_class)
	{
		SOMClass_somRelease(data->explicit_meta_class);
	}
	while (data->seq._length--)
	{
		if (data->seq._buffer[data->seq._length])
		{
			SOMClass_somRelease(data->seq._buffer[data->seq._length]);
		}
	}
#endif
	if (data->failed)
	{
#ifdef SOMClass_somRelease
		if (data->classObject)
		{
			SOMClass_somRelease(data->classObject);
		}
#else
		if (data->classObject && data->was_created)
		{
			SOMClass_somFree(data->classObject);
		}
#endif
		data->classObject=NULL;
	}

RHBOPT_cleanup_end

SOMClass SOMSTAR SOMLINK somBuildClass (
                    long inherit_vars,
                    somStaticClassInfo *sci,
                    long majorVersion,
                    long minorVersion)
{
	struct somBuildClass data={0,0,0,NULL,NULL,NULL,{0,0,NULL},
				{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}};

	SOM_IgnoreWarning(majorVersion);
	SOM_IgnoreWarning(minorVersion);

	RHBOPT_cleanup_push(somBuildClass_cleanup,&data);

	if (sci)
	{
		if (sci->layoutVersion <= 4)
		{
			if (sci->cds->classObject)
			{
#ifdef SOMClass_somDuplicateReference
				data.classObject=SOMClass_somDuplicateReference(sci->cds->classObject);
#else
				data.classObject=sci->cds->classObject;
#endif
			}
			else
			{
				if (sci->numParents < (sizeof(data.parents)/sizeof(data.parents[0])))
				{
					somMethodTabPtr nc=NULL;

					data.seq._buffer=data.parents;
					data.seq._maximum=sci->numParents;

					while (data.seq._length < data.seq._maximum)
					{
						data.parents[data.seq._length]=SOMClassMgr_somFindClass(
							SOMClassMgrObject,
							(somId)sci->parents[data.seq._length],0,0);

						if (!data.parents[data.seq._length])
						{
							data.failed=1;
							break;
						}

						data.seq._length++;
					}

					if (sci->explicitMetaId)
					{
						data.explicit_meta_class=SOMClassMgr_somFindClass(SOMClassMgrObject,
								(somId)sci->explicitMetaId,0,0);

						if (!data.explicit_meta_class)
						{
							data.failed=1;
						}
					}

/*					if ((!data.meta_class) && (!data.failed))
					{
						if (data.seq._length)
						{
							data.meta_class=SOMObject_somGetClass(data.seq._buffer[0]);
						}
					}

					if ((!data.meta_class)&&(!data.failed))
					{
#ifdef SOMClass_somDuplicateReference
						data.meta_class=SOMClass_somDuplicateReference(SOMClassClassData.classObject);
#else
						data.meta_class=SOMClassClassData.classObject;
#endif
					}
					*/

					if (!data.failed)
					{
						somStartCriticalSection();

						data.locked=1;

						data.meta_class=SOMClass_somClassOfNewClassWithParents(
								somStringFromId((somId)sci->classId),
								&data.seq,data.explicit_meta_class);

						if (!data.meta_class) data.failed=1;
					}

					if (data.meta_class && !data.failed)
					{
						if (sci->cds->classObject)
						{
							/* during the preparation for construction it was built */
#ifdef SOMClass_somDuplicateReference
							data.classObject=SOMClass_somDuplicateReference(sci->cds->classObject);
#else
							data.classObject=sci->cds->classObject;
#endif
						}
						else
						{
							if (data.meta_class)
							{
								data.classObject=SOMClass_somNew(data.meta_class);
							}
							else
							{
								data.failed=1;
							}

							if (data.classObject && !data.failed)
							{
								long dataAlignment=(sci->layoutVersion > 1) ?
									sci->dataAlignment
									:
									(long)&(((struct { octet _d; octet _v; } *)0)->_v);

/*			somPrintf("somBuildClass(%s) - begin\n",somStringFromId((somId)sci->classId));*/

								data.was_created=1;

								data.failed=1;

								SOMClass_somInitMIClass(
										data.classObject,
										inherit_vars,
										somStringFromId((somId)sci->classId),
										&data.seq,
										sci->instanceDataSize,
										dataAlignment,
										SOMKERN_total_defined_methods(sci),
										sci->majorVersion,
										sci->minorVersion);

								if (sci->ccds)
								{
									sci->ccds->instanceDataToken=
										SOMClass_somGetInstanceToken(data.classObject);
								}

								SOMClass_somSetClassData(data.classObject,sci->cds);

								nc=SOMClass_somGetClassMtab(data.classObject);

								somClassInfoFromMtab(nc)->sci=sci;

								/* need to have the parentMtab setup here prior
									to the cif being called */

								sci->ccds->parentMtab=&(somClassInfoFromMtab(nc)->parents);

								if (sci->numStaticMethods)
								{
									somStaticMethod_t *smt=sci->smt;
									unsigned long i=sci->numStaticMethods;

									while (i--)
									{
										somMToken m=SOMClass_somAddStaticMethod(data.classObject,
												(somId)*(smt->methodId),
												(somId)*(smt->methodDescriptor),
												smt->method,
												smt->redispatchStub,
												smt->applyStub);
#ifdef SOM_METHOD_STUBS
										smt->classData[0]=(somMToken)&(SOMMTOKEN_DATA(m)->jumper.direct);
#else
										smt->classData[0]=m;
#endif
										m->defined.classData=smt->classData;
										smt++;
									}
								}

								if (sci->numStaticOverrides)
								{
									somOverrideMethod_t *omt=sci->omt;
									unsigned int i=sci->numStaticOverrides;

									while (i--)
									{
										SOMClass_somOverrideSMethod(data.classObject,(somId)*(omt->methodId),omt->method);
										omt++;
									}
								}

								if (sci->numProcMethods)
								{
									somProcMethods_t *pmt=sci->pmt;
									unsigned int i=sci->numProcMethods;

									while (i--)
									{
										pmt->classData[0]=pmt->pEntry;

										pmt++;
									}
								}

								if (sci->numVarargsFuncs)
								{
									unsigned long i=sci->numVarargsFuncs;
									somVarargsFuncs_t *pva=sci->vft;

									while (i--)
									{
										(*(pva->classData))=pva->vEntry;

										pva++;
									}
								}

								if (sci->cif)
								{
									sci->cif(data.classObject);
								}

								data.failed=0;

								SOMClass_somClassReady(data.classObject);

								RHBOPT_ASSERT(!sci->cds->classObject);

								sci->cds->classObject=data.classObject;


/*			somPrintf("somBuildClass(%s) - end\n",somStringFromId((somId)sci->classId));*/
							}
						}
					}
				}
			}
		}
	}

	RHBOPT_cleanup_pop();

	return data.classObject;
}

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		static pthread_once_t SOMKERN_once=RHBOPT_PTHREAD_ONCE_INIT;
		static void SOMKERN_once_r(void)
		{
			SOM_dll_init();
		}
		#define SOM_THREAD_INIT_ONCE	pthread_once(&SOMKERN_once,SOMKERN_once_r);
	#else
		#ifdef _WIN32
			/* DllMain will have done the job */
			#define SOM_THREAD_INIT_ONCE
		#else
			#error not ported to this threading architecture
		#endif
	#endif
#else
	#define SOM_THREAD_INIT_ONCE
#endif

#ifdef USE_THREADS
static som_thread_globals_t *make_globals(void)
{
	som_thread_globals_t *ev=(som_thread_globals_t *)SOMCalloc(sizeof(*ev),1);
	if (ev) 
	{
		ev->somOutCharRoutine=SOMOutCharRoutine;
	}
	return ev;
}
#endif

static som_thread_globals_t *SOMKERN_get_thread_globals(char make)
{
#ifdef USE_THREADS
	som_thread_globals_t *ev;

	SOM_THREAD_INIT_ONCE

	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_GETSPECIFIC_STD
			ev=(som_thread_globals_t *)pthread_getspecific(som_globals.tls_key);
		#else
		{
			pthread_addr_t pv;
			if (!pthread_getspecific(som_globals.tls_key,&pv))
			{
				ev=(som_thread_globals_t *)pv;
			}
			else
			{
				ev=NULL;
			}
		}
		#endif
		if (!ev && make)
		{
			ev=make_globals();
#ifdef HAVE_PTHREAD_ADDR_T
			pthread_setspecific(som_globals.tls_key,(pthread_addr_t)(void *)ev);
#else
			pthread_setspecific(som_globals.tls_key,ev);
#endif
		}
	#else
			ev=(som_thread_globals_t *)TlsGetValue(som_globals.tls_key);
			if (!ev && make)
			{
				ev=make_globals();
				TlsSetValue(som_globals.tls_key,ev);
			}
	#endif
	return ev;
#else
	#ifdef RHBOPT_SHARED_DATA
		som_thread_globals_t *ev;
#ifdef _WIN32
		DWORD tid=GetCurrentThreadId();
#else
		ProcessSerialNumber psn;
		if (GetCurrentProcess(&psn)) 
		{
			somPrintf("GetCurrentProcess failed,%s:%d\n",__FILE__,__LINE__);
			return NULL;
		}
#endif
		ev=som_globals.apps;
		while (ev)
		{
#ifdef _WIN32
			if (tid==ev->tid) break;
#else
			Boolean b=0;

			if (!SameProcess(&ev->psn,&psn,&b))
			{
				if (b) break;
			}
#endif

			ev=ev->next;
		}

		if (!ev)
		{
			if (make)
			{
				somPrintf("making app environment\n");
				ev=SOMCalloc(sizeof(*ev),1);
#ifdef _WIN32
				ev->tid=tid;
#else
				ev->psn=psn;
#endif
				ev->next=som_globals.apps;
				som_globals.apps=ev;

				ev->somError=SOMKERN_error;
				ev->somOutCharRoutine=SOMKERN_outchar;
			}
		}

		return ev;
	#else
		static som_thread_globals_t ev;
	
		return &ev;
	#endif
#endif
}

Environment * SOMLINK somGetGlobalEnvironment(void)
{
som_thread_globals_t *ev=SOMKERN_get_thread_globals(1);

	if (ev)
	{
		return &ev->ev;
	}

	return 0;
}

somMethodProc * SOMLINK somParentResolve(somMethodTabs parentMtabs,
                                         somMToken mToken)
{
/*	SOMKERN_check_mem();*/

	return somParentNumResolve(parentMtabs,1,mToken);
}

somMethodProc * SOMLINK somParentNumResolve(somMethodTabs parentMtabs,
                                         int parentNum,
                                         somMToken mToken)
{
#if 1
	/* new way which allows shadowed/substituted classes to change mtab */
	struct somMethodTabStruct *guess=somClassInfoFromParentMtabs(parentMtabs)->parent_jump_table._buffer[parentNum-1];
#else
	struct somMethodTabStruct *guess=somClassInfoFromMtab(parentMtabs->mtab)->parent_jump_table._buffer[parentNum-1];
#endif

#ifdef _DEBUG
	somClassInfo c1=somClassInfoFromMtab(parentMtabs->mtab);
	somClassInfo c2=somClassInfoFromParentMtabs(parentMtabs);

	RHBOPT_ASSERT(c1==c2);
	RHBOPT_ASSERT(parentMtabs==(somMethodTabs)&c1->parents);
	RHBOPT_ASSERT((parentNum-1)<(int)(c1->numParents));
#endif

	RHBOPT_ASSERT(parentNum > 0);

#ifdef SOM_METHOD_THUNKS
	RHBOPT_ASSERT(mToken);
	mToken=SOMMTOKEN_DATA(mToken);
	RHBOPT_ASSERT(mToken==SOMMTOKEN_DATA(mToken));
#endif

	RHBOPT_ASSERT(mToken!=SOMObjectClassData.somInit)
	RHBOPT_ASSERT(mToken!=SOMObjectClassData.somUninit)

	return SOMKERN_resolve(guess,mToken);
}

somMethodProc * SOMLINK somClassResolve(SOMClass SOMSTAR cls, somMToken mToken)
{
	somMethodTab *mtab=SOMClass_somGetClassMtab(cls);

#ifdef SOM_METHOD_THUNKS
	RHBOPT_ASSERT(mToken);
	mToken=SOMMTOKEN_DATA(mToken);
	RHBOPT_ASSERT(mToken==SOMMTOKEN_DATA(mToken));
#endif

	return SOMKERN_resolve(mtab,mToken);
}

somMethodProc * SOMLINK somAncestorResolve(SOMObject SOMSTAR obj, /* the object */
                                           somCClassDataStructure *ccds, /* id the ancestor */
                                           somMToken mToken)
{
	SOM_IgnoreWarning(obj);
	SOM_IgnoreWarning(ccds);
	SOM_IgnoreWarning(mToken);

	RHBOPT_ASSERT(obj && !obj)

	return NULL;
}

SOMEXTERN somToken SOMLINK somDataResolveChk(
			SOMObject SOMSTAR obj, 
			somDToken dataId)
{
	unsigned int l;
	somMethodTabPtr p=somMethodTabFromObject(obj);

	if (somClassInfoFromMtab(p)->multiple_inheritance)
	{
		struct somParentClassInfo *q=somClassInfoFromMtab(p)->classes._buffer;
		l=somClassInfoFromMtab(p)->classes._length;

		while (q->cls != dataId->cls)
		{
			q++;
			l--;
			if (!l) return NULL;
		}

		l=q->data_offset;
	}
	else
	{
		l=dataId->data_offset;
	}

	if (l)
	{
		return (char *)(((char *)obj)+l);
	}

	return NULL;
}

static struct somMTokenData *SOMKERN_somMToken_by_name(somClassInfo info,somId id)
{
	unsigned int i=info->added_methods._length;

	if (i)
	{
		struct somMTokenData *_buffer=info->added_methods._buffer;
		const char *p=somStringFromId(id);
		boolean has_colon=0;
		while (*p) 
		{ 
			if (*p++==':') 
			{
				has_colon=1;

				break ;
			}
		}

		if (has_colon)
		{
			while (i--)
			{
				if (somCompareIds(
					(somId)(_buffer->defined.somId_methodDescriptor),
						id))
				{
					return _buffer;
				}

				_buffer++;
			}
		}
		else
		{
			while (i--)
			{
				if (somCompareIds(
					(somId)(_buffer->defined.somId_methodId),
						id))
				{
					return _buffer;
				}

				_buffer++;
			}
		}
	}

	i=info->numParents;

	if (i)
	{
		while (i--)
		{
			struct somMTokenData *_buffer=SOMKERN_somMToken_by_name(
				somClassInfoFromMtab(info->parent_jump_table._buffer[i]),id);

			if (_buffer) return _buffer;
		}
	}

	return NULL;
}

somMethodProc * SOMLINK somResolveByName(
		SOMObject SOMSTAR obj,
        char *methodName)
{
	somMToken m=SOMKERN_somMToken_by_name(
		somClassInfoFromMtab(somMethodTabFromObject(obj)),
			&methodName);

	if (m) return somResolve(obj,m);

	return NULL;
}

int SOMLINK somPrintf(const char * fmt, ...)
{
#if defined(RHBOPT_SHARED_DATA) || defined(USE_THREADS)
som_thread_globals_t *tev=SOMKERN_get_thread_globals(0);
somTD_SOMOutCharRoutine *local_SOMOutCharRoutine=tev ? tev->somOutCharRoutine : SOMOutCharRoutine;
#endif
	int i=0;

	if (local_SOMOutCharRoutine)
	{
		char buf[4096];
		int j;

		va_list arg;

		somStartCriticalSection();

		va_start(arg,fmt);

#ifdef HAVE_VSNPRINTF
		i=vsnprintf(buf,sizeof(buf),fmt,arg);
#else
	#ifdef HAVE_INT_VSPRINTF
		i=vsprintf(buf,fmt,arg);
	#else
		vsprintf(buf,fmt,arg);
		i=strlen(buf);
	#endif
#endif
		va_end(arg);

		j=0;

		while (j < i)
		{
			local_SOMOutCharRoutine(buf[j]);

			j++;
		}

		local_SOMOutCharRoutine(0);

		somEndCriticalSection();
	}

	return i;
}

/*boolean  SOMLINK somva_SOMObject_somDispatch(SOMObject SOMSTAR somSelf,
		somToken* retValue,
		somId methodId,
		...)
{
	int i;
	va_list arg;
	va_start(arg,methodId);
	i=SOMObject_somDispatch(somSelf,retValue,methodId,arg);
	va_end(arg);
	return i;
}
*/

static void *SOMKERN_suballoc_v(_IDL_SEQUENCE_octet *seq,size_t len,short align)
{
	long l=(long)seq->_length;
	octet *op=NULL;

	RHBOPT_ASSERT(align);
	RHBOPT_ASSERT(len);

	if (l & (align-1))
	{
		l=(l+align)&(long)(-align);
	}

	RHBOPT_ASSERT(l >= ((long)seq->_length));
	RHBOPT_ASSERT(!(l & (align-1)));

	if (seq->_buffer)
	{
		op=seq->_buffer+l;
	}

	seq->_length=(long)(l+len);

	if (seq->_buffer)
	{
		RHBOPT_ASSERT((seq->_length <= seq->_maximum));
	}

	return op;
}

#define _SOMKERN_suballoc(a,n,t) (void)SOMKERN_suballoc_v(a,n,SOMKERN_alignof(t))
#define SOMKERN_suballoc(a,n,t)  (t *)SOMKERN_suballoc_v(a,n,SOMKERN_alignof(t))

#define SOMKERN_somClassInfo_SI   0

static struct somClassInfoMethodTab * SOMKERN_allocate_somMethodTab(
			const char *p,
			boolean multiply_inherited,
			unsigned long immediate_parentClasses,
			unsigned long total_parentClasses,
			unsigned long total_static_methods,
			unsigned long added_static_methods,
			somStaticClassInfo *sci,
			long major_v,
			long minor_v,
			long dataSize,
			long dataAlignment,
			long maxCtrlMask)
{
	int name_len=(int)strlen(p);
	_IDL_SEQUENCE_octet alloc={0,0,NULL};
	somMethodTabPtr info;
	struct somClassInfoMethodTab *classInfoMtab;
	somClassInfo classInfo;

	total_parentClasses++; /* to include this one */

	/* order of entries is important for correct alignment
		
		somClassInfoMethodTab up to mtab.entries
		entries[total_static_methods]
		added_methods
		parent_jump_table[immediate_parentClasses]
		classes[total_parentClasses]
		initCtrl.info[total_parentClasses]
		assignCtrl.info[total_parentClasses]
		destructCtrl.info[total_parentClasses]
		initCtrl.mask[maxCtrlMask]
		assignCtrl.mask[maxCtrlMask]
		destructCtrl.mask[maxCtrlMask]
		className[nameLen+1]
	*/

	_SOMKERN_suballoc(&alloc,
					SOMKERN_offsetof(struct somClassInfoMethodTab,mtab.entries),
					struct somClassInfoMethodTab);

	_SOMKERN_suballoc(&alloc,
					sizeof(info->entries[0])*total_static_methods,
					somMethodPtr);

	if (added_static_methods)
	{
		_SOMKERN_suballoc(&alloc,
					sizeof(classInfo->added_methods._buffer[0])*added_static_methods,
					struct somMTokenData);
	}

	_SOMKERN_suballoc(&alloc,
					sizeof(info->classInfo->parent_jump_table._buffer[0])*immediate_parentClasses,
					struct somMethodTabStruct);

	_SOMKERN_suballoc(&alloc,
					sizeof(classInfo->classes._buffer[0])*total_parentClasses,
					struct somParentClassInfo);

	_SOMKERN_suballoc(&alloc,
					sizeof(classInfo->parents.initCtrl.info[0])*total_parentClasses,
					somInitInfo);

	_SOMKERN_suballoc(&alloc,
					sizeof(classInfo->parents.assignCtrl.info[0])*total_parentClasses,
					somAssignInfo);

	_SOMKERN_suballoc(&alloc,
					sizeof(classInfo->parents.destructCtrl.info[0])*total_parentClasses,
					somDestructInfo);

	/* these are all octets */

	_SOMKERN_suballoc(&alloc,(maxCtrlMask << 1)+maxCtrlMask+name_len+1,octet);

	alloc._maximum=alloc._length;
	alloc._buffer=SOMMallocEx(alloc._length);
	memset(alloc._buffer,0,alloc._length);
	alloc._length=0;

						
	classInfoMtab=
		SOMKERN_suballoc(&alloc,
						SOMKERN_offsetof(struct somClassInfoMethodTab,mtab.entries),
						struct somClassInfoMethodTab);

	info=&classInfoMtab->mtab;
	classInfo=&classInfoMtab->classInfo;

	info->classInfo=classInfo; 

	_SOMKERN_suballoc(&alloc,
					sizeof(info->entries[0])*total_static_methods,
					somMethodPtr);

	RHBOPT_ASSERT((multiply_inherited & ~1)==0)

	classInfo->multiple_inheritance=multiply_inherited;
	classInfo->jump_table._maximum=total_static_methods;
	classInfo->major_version=major_v;
	classInfo->minor_version=minor_v;
	classInfo->sci=sci;
	classInfo->max_mask_length=(short)maxCtrlMask;

	info->dataAlignment=dataAlignment;

	if (added_static_methods)
	{
		classInfo->added_methods._maximum=added_static_methods;

		classInfo->added_methods._buffer=SOMKERN_suballoc(&alloc,
			sizeof(classInfo->added_methods._buffer[0])*added_static_methods,
			struct somMTokenData);
	}

	classInfo->parent_jump_table._maximum=immediate_parentClasses;
	classInfo->parent_jump_table._buffer=SOMKERN_suballoc(&alloc,
		sizeof(info->classInfo->parent_jump_table._buffer[0])*immediate_parentClasses,
		struct somMethodTabStruct *);

	classInfo->classes._maximum=total_parentClasses;
	classInfo->classes._buffer=SOMKERN_suballoc(&alloc,
		sizeof(classInfo->classes._buffer[0])*total_parentClasses,
		struct somParentClassInfo);

	/* allocate the constructor infos in */

	classInfo->parents.initCtrl.info=SOMKERN_suballoc(&alloc,
		sizeof(classInfo->parents.initCtrl.info[0])*total_parentClasses,
		somInitInfo);

	classInfo->parents.assignCtrl.info=SOMKERN_suballoc(&alloc,
		sizeof(classInfo->parents.assignCtrl.info[0])*total_parentClasses,
		somAssignInfo);

	classInfo->parents.destructCtrl.info=SOMKERN_suballoc(&alloc,
		sizeof(classInfo->parents.destructCtrl.info[0])*total_parentClasses,
		somDestructInfo);

	classInfo->parents.initCtrl.infoSize=sizeof(classInfo->parents.initCtrl.info[0]);
	classInfo->parents.assignCtrl.infoSize=sizeof(classInfo->parents.assignCtrl.info[0]);
	classInfo->parents.destructCtrl.infoSize=sizeof(classInfo->parents.destructCtrl.info[0]);

	/* allocate the constructor masks, these are somBooleanVectors */

	classInfo->parents.initCtrl.mask=SOMKERN_suballoc(&alloc,maxCtrlMask,octet);
	classInfo->parents.assignCtrl.mask=SOMKERN_suballoc(&alloc,maxCtrlMask,octet);
	classInfo->parents.destructCtrl.mask=SOMKERN_suballoc(&alloc,maxCtrlMask,octet);

	info->className=SOMKERN_suballoc(&alloc,name_len+1,char);
	memcpy(info->className,p,name_len+1);

	RHBOPT_ASSERT(alloc._length==alloc._maximum);

	classInfo->instancePartSize=dataSize;
	classInfo->dataAlignment=dataAlignment;

	classInfo->keyed_data.key._buffer=(octet *)info->className;
	classInfo->keyed_data.key._length=(long)strlen(info->className);
	classInfo->keyed_data.key._maximum=classInfo->keyed_data.key._length;
	classInfo->keyed_data.data._value=info;

	return classInfoMtab;
}

static struct somParentClassInfo *SOMKERN_get_this_class(somMethodTabPtr p)
{
	unsigned int i=0;
	somClassInfo info=somClassInfoFromMtab(p);
	struct somParentClassInfo *b=info->classes._buffer;

	while (i < info->classes._length)
	{
		if (b->cls==p)
		{
			return b;
		}
		b++;

		i++;
	}

/*	somPrintf("Class %s is not in it's own list\n",p->className);*/

	RHBOPT_ASSERT(i<info->classes._maximum);

	if (i==info->classes._maximum)
	{
		SOMError(-1,__FILE__,__LINE__);

		return NULL;
	}

	b->cls=p;
	b->inherit_var=1;
	info->classes._length++;

	return b;
}

static int SOMKERN_is_class_in(somMethodTabPtr *p,unsigned int i,somMethodTabPtr c)
{
	while (i--)
	{
		if (*p == c)
		{
			return 1;
		}
		else
		{
			somMethodTabPtr q=*p++;
			somClassInfo info=somClassInfoFromMtab(q);
			unsigned int k=info->classes._length;

			while (k--)
			{
				if (c==info->classes._buffer[k].cls)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

static void SOMKERN_copy_somMethodTab(
			boolean inherit_var,	/* whether to inherit implementation */
			somMethodTabPtr info,
			somMethodTabPtr from)
{
	unsigned int i=0;
	somClassInfo cinfo=somClassInfoFromMtab(info);
	somClassInfo finfo=somClassInfoFromMtab(from);
	struct somParentClassInfo *parents=cinfo->classes._buffer;

	if (cinfo->classes._length)
	{
		parents+=cinfo->classes._length;
	}

	/* for multiple inheritence, should confirm that the
		same class does not exist twice in this list */

	while (i < finfo->classes._length)
	{
		if (!SOMKERN_is_class_in(&info,1,finfo->classes._buffer[i].cls))
		{
			*parents=finfo->classes._buffer[i];

			/* this indicates how it is included in this MI class */
			parents->inherit_var=inherit_var;  
			parents->jump_table_offset=(unsigned long)~0L;

			parents++;
			cinfo->classes._length++;
		}

		i++;
	}

	i=cinfo->classes._length;
	parents=cinfo->classes._buffer;

	while (i--)
	{
		if (parents->jump_table_offset==~0L)
		{
			unsigned int k;
			somMethodPtr p;
			somMToken t;
			struct somMTokenData *je;
			/* has no jump table */

/*			somPrintf("%s += %s::%s\n",
				info->className,
				from->className,
				parents->cls->className);
*/					

			parents->jump_table_offset=cinfo->jump_table._length;

			k=somClassInfoFromMtab(parents->cls)->added_methods._length;
			je=somClassInfoFromMtab(parents->cls)->added_methods._buffer;

			while (k--)
			{
				t=je;

				if (inherit_var)
				{
					p=SOMKERN_resolve(from,t);
				}
				else
				{
					p=t->defined.redispatchStub;
				}

#ifdef SOM_DEBUG_JUMP_TABLE
				info->jump_table_buffer[info->jump_table._length].method=p;
				info->jump_table_buffer[info->jump_table._length].mToken=t;
#else
				info->entries[cinfo->jump_table._length]=p;
#endif
/*				{
					char *p;

					p=somStringFromId(*t->defined_in->methodDescriptor);
					somPrintf("added %s to %s at [%d]\n",
							p,info->className,
							(int)info->jump_table._length);
				}*/

				cinfo->jump_table._length++;

				je++;
			}
		}

		parents++;
	}
}

static void SOMKERN_AllocateInstanceSize(struct somClassInfoMethodTab *cimtab)
{
	somClassInfo info=&cimtab->classInfo;
	struct somMethodTabStruct *m=&cimtab->mtab;
	long l=sizeof(somMethodTabPtr);
	unsigned int i=0;

	/* there should really be some alignment business going on here....
		taking into acount the alignment wanted by SCI */

	while (i < info->classes._length)
	{
		if (info->classes._buffer[i].inherit_var)
		{
			somClassInfo inf2=somClassInfoFromMtab(info->classes._buffer[i].cls);
			long k=inf2->instancePartSize;

			if (k)
			{
				long a=inf2->dataAlignment;

				if (a > 1)
				{
					long mask=(a-1);
					l=(l+mask)&(~mask);
				}

				info->classes._buffer[i].data_offset=(SOMKERN_data_index_t)l;

				l+=k;
			}
		}
		else
		{
			info->classes._buffer[i].data_offset=0;
		}

		i++;
	}

	i--;

	/* don't generate a data token if no data for class */

	if (info->classes._buffer[i].data_offset)
	{
#ifdef SOM_DATA_THUNKS
		info->data_token.jump_data=SOMObjectCClassData.instanceDataToken->jump_data;
#endif
		info->data_token.cls=m;
		info->data_token.data_offset=info->classes._buffer[i].data_offset;
	}

	m->instanceSize=l;
}

SOMEXTERN char * SOMLINK somStringFromId (somId id)
{
	if (id)
	{
		return *id;
	}

	return NULL;
}

static int SOMKERN_any_mi(somMethodTabPtr p)
{
	somClassInfo info=somClassInfoFromMtab(p);

	if (info->multiple_inheritance)
	{
		return 1;
	}
	else
	{
		unsigned int i=info->classes._length;

		while (i--)
		{
			if (somClassInfoFromMtab(info->classes._buffer[i].cls)->multiple_inheritance)
			{
				return 1;
			}
		}	
	}

	return 0;
}

static int SOMKERN_count_unique_classes(somMethodTabPtr *p,unsigned int i)
{
	int total=0;

	while (i--)
	{
		somMethodTabPtr q=*p++;
		somClassInfo info=somClassInfoFromMtab(q);
		unsigned int k=info->classes._length;

		while (k--)
		{
			somMethodTabPtr x=info->classes._buffer[k].cls;

			if (!SOMKERN_is_class_in(p,i,x))
			{
/*				somPrintf("new class %s\n",x->className);*/
				total++;
			}
		}
	}

	return total;
}

static int SOMKERN_count_unique_methods(somMethodTabPtr *p,unsigned int i)
{
	int total=0;

	while (i--)
	{
		somMethodTabPtr q=*p++;
		unsigned int k=somClassInfoFromMtab(q)->jump_table._length;

		while (k--)
		{
			if (!SOMKERN_is_method_in(p,i,
				/*	q->jump_table._buffer[k].mToken*/
					SOMKERN_index_to_somMToken(q,k)
					))
			{
				total++;
			}
		}
	}

	return total;
}

static int SOMKERN_is_method_in(somMethodTabPtr *q,unsigned int i,somMToken t)
{
/*	{
		char *r;

		r=somStringFromId(*(t->defined_in->methodDescriptor));

		somPrintf("Checking for %s\n",r);
	}
*/
	while (i--)
	{
		somMethodTabPtr p=*q++;
		somClassInfo info=somClassInfoFromMtab(p);
		struct somParentClassInfo *par=info->classes._buffer;
		unsigned int j=info->classes._length;

		/* this is Rhubarb, should actually
			get class the method is implemented in 
			and then zip through class list */

		while (j--)
		{
			if (par->cls==t->classInfoOwner)
			{
				unsigned int k=SOMKERN_resolve_index(p,t);
				somMethodPtr *je=p->entries+k;

#ifdef SOM_DEBUG_JUMP_TABLE
				if (je->method)
				{
					return 1;
				}
#else
				RHBOPT_ASSERT(*je)

				if (*je)
				{
					return 1;
				}
#endif
			}

			par++;
		}

/*		{
			char *r;

			r=somStringFromId(*(t->defined_in->methodDescriptor));

			somPrintf("%s does not contain %s\n",
					p->className,r);
		}*/
	}

	return 0;
}


static void SOMKERN_set_up_parents(somMethodTabPtr mtab,somMethodTabPtr *parents,int num)
{
	somClassInfo classInfo=somClassInfoFromMtab(mtab);

/*	somPrintf("setting up parent list for %s and %d parents\n",
				mtab->className,
				num);*/

	RHBOPT_ASSERT(mtab==somMtabFromClassInfo(classInfo));

	classInfo->parents.mtab=mtab;
	classInfo->parents.classObject=mtab->classObject;
	classInfo->parents.next=NULL;

	if (num)
	{
		struct somParentClassInfo * p=SOMKERN_get_info_for(mtab,*parents);

		if (p)
		{
			int i=0;

			while (i < num)
			{
				/* self should not be in this list */
				RHBOPT_ASSERT(parents[i]!=mtab)

				classInfo->parent_jump_table._buffer[i]=parents[i];

				i++;
			}

			classInfo->parent_jump_table._length=i;

			parents++;
			num--;

			classInfo->parents.next=&p->list_entry;

			while (num--)
			{
				struct somParentClassInfo *q=SOMKERN_get_info_for(mtab,*parents);

				parents++;

				p->list_entry.next=&q->list_entry;

				p=q;
			}
		}
	}
}

static struct somParentClassInfo * SOMKERN_get_info_for(somMethodTabPtr mtab,somMethodTabPtr parent)
{
	somClassInfo info=somClassInfoFromMtab(mtab);
	unsigned int i=info->classes._length;
	struct somParentClassInfo *b=info->classes._buffer;

	while (i--)
	{
		if (b->cls==parent) 
		{
			b->list_entry.mtab=parent;

			return b;
		}

		b++;	
	}

	RHBOPT_ASSERT(!parent);

	return NULL;
}

static unsigned long SOMKERN_total_defined_methods(somStaticClassInfo *sci)
{
	unsigned long n;

	n=sci->maxMethods;

#ifdef USE_SOM21
	if (sci->layoutVersion > 3)
	{
		n=sci->numStaticMethods+sci->numMethods;

		if (n < sci->maxMethods)
		{
			n=sci->maxMethods;
		}
	}
#endif

	return n;
}


static void SOMKERN_UnbootStrap(void)
{
	som_thread_globals_t *ev=ev=SOMKERN_get_thread_globals(0);

	if (ev)
	{
#ifdef USE_THREADS
		SOMKERN_end_thread(ev);
	#ifdef USE_PTHREADS
		pthread_setspecific(som_globals.tls_key,NULL);
	#else
		TlsSetValue(som_globals.tls_key,NULL);
	#endif
#endif
	}

#ifdef RHBOPT_SHARED_DATA
	if (!som_globals.apps)
#endif
	{
	#ifdef SOM_DEBUG_MEMORY
		som_dump_mem();
	#endif

		/* this bombed when calling pthread_key_delete()
			on linux 2.0.30 with glibc 2.0.6 and linuxthread0.7 */
	#ifdef SOMKERN_DLL_TERM
		SOM_dll_term();
	#endif
	}
}

SOMEXTERN void SOMLINK somRegisterClassLibrary (char * libraryName,
    somMethodProc *libraryInitRtn)
{
	/* points to a "SOMInitModule" type function */

	somPrintf("SOM.DLL somRegisterClassLibrary not implemented\n");

	SOM_IgnoreWarning(libraryName);
	SOM_IgnoreWarning(libraryInitRtn);
}

SOMEXTERN boolean SOMLINK somAbnormalEnd (void)
{
	somPrintf("SOM.DLL somAbnormalEnd not implemented\n");

	return 0;
}

SOMEXTERN void SOMLINK somSetOutChar(somTD_SOMOutCharRoutine *outch)
{
#if defined(RHBOPT_SHARED_DATA) || defined(USE_THREADS)
	if (outch)
	{
		som_thread_globals_t *tev=SOMKERN_get_thread_globals(1);
		if (tev) tev->somOutCharRoutine=outch;
	}
	else
	{
		som_thread_globals_t *tev=SOMKERN_get_thread_globals(0);
		if (tev) tev->somOutCharRoutine=outch;
	}
#else
	SOMOutCharRoutine=outch;
#endif
}


SOMEXTERN void SOMLINK somConstructClass (
                    somTD_classInitRoutine *classInitRoutine,
                    SOMClass SOMSTAR parentClass,
                    SOMClass SOMSTAR metaClass,
                    somClassDataStructure *cds)
{
	somPrintf("SOM.DLL somConstructClass not implemented\n");

	SOM_IgnoreWarning(classInitRoutine);
	SOM_IgnoreWarning(parentClass);
	SOM_IgnoreWarning(metaClass);
	SOM_IgnoreWarning(cds);
}

SOMEXTERN SOMObject SOMSTAR SOMLINK somTestCls(
		SOMObject SOMSTAR obj, 
		SOMClass SOMSTAR classObj,
        const char * fileName, int lineNumber)
{
	if (obj)
	{
		if (classObj)
		{
			somMethodTabPtr m=somMethodTabFromObject(obj);
			somClassInfo info=somClassInfoFromMtab(m);
			unsigned int i=info->classes._length;

			while (i--)
			{
				if (info->classes._buffer[i].cls->classObject==classObj)
				{
					return obj;
				}
			}
		}
	}

	return NULL;
}

SOMEXTERN void SOMLINK somStartCriticalSection(void)
{
#ifdef USE_THREADS
#	ifdef USE_RHBMUTEX_LOCK
		RHBMUTEX_LOCK(&som_global_mutex);
#	else
		EnterCriticalSection(&som_global_mutex);
#	endif
#else
	/* sigset stuff */

	#if defined(_PLATFORM_MACINTOSH_) || defined(RHBOPT_SHARED_DATA) || defined(_WIN32S)

	#else
	static sigset_t *gBlockedSignalMask;

	#if defined(_WIN32) && !defined(_WIN32S)
	if (som_tidMain!=GetCurrentThreadId()) return;
	#endif

	if (!gBlockedSignalMask)
	{
		static sigset_t gBlockedSignalMaskData;
		sigset_t blockSignalMask;

		/* mask out all async signals, not SIGFPE,SIGILL,SIGSYS etc */

		sigemptyset(&blockSignalMask);
		sigaddset(&blockSignalMask,SIGHUP);
		sigaddset(&blockSignalMask,SIGINT);
		sigaddset(&blockSignalMask,SIGQUIT);
		sigaddset(&blockSignalMask,SIGTERM);
		sigaddset(&blockSignalMask,SIGALRM);
#ifdef SIGCLD
		sigaddset(&blockSignalMask,SIGCLD);
#else
		sigaddset(&blockSignalMask,SIGCHLD);
#endif
#ifdef SIGIO
		sigaddset(&blockSignalMask,SIGIO);
#endif

		gBlockedSignalMaskData=blockSignalMask;
		gBlockedSignalMask=&gBlockedSignalMaskData;
	}

	if (sigprocmask(SIG_BLOCK,gBlockedSignalMask,som_critical_masks+som_critical_count))
	{
#ifdef SIGSYS
		raise(SIGSYS);
#else
		raise(SIGABRT);
#endif
	}
	else
	{
#ifdef somStartCriticalSection
		debugCritical[som_critical_count].file=file;
		debugCritical[som_critical_count].line=line;
#endif
		if (som_critical_count < (sizeof(som_critical_masks)/sizeof(som_critical_masks[0])))
		{
			som_critical_count++;
		}
		else
		{
#ifdef somStartCriticalSection
			unsigned int i=0;
			while (i < som_critical_count)
			{
				printf("%s:%d\n",
					debugCritical[i].file,
					debugCritical[i].line);
				i++;
			}
			__asm int 3;
#endif
#ifdef SIGSYS
			raise(SIGSYS);
#else
			raise(SIGABRT);
#endif
		}
	}
	#endif
#endif
}

SOMEXTERN void SOMLINK somEndCriticalSection(void)
{
#ifdef USE_THREADS
#	ifdef USE_RHBMUTEX_LOCK
		RHBMUTEX_UNLOCK(&som_global_mutex);
#	else
		LeaveCriticalSection(&som_global_mutex);
#	endif
#else
	#if defined(_PLATFORM_MACINTOSH_) || defined(RHBOPT_SHARED_DATA) || defined(_WIN32S)
	#else
		#if defined(_WIN32) && !defined(_WIN32S)
			if (som_tidMain!=GetCurrentThreadId()) return;
		#endif

		if (som_critical_count)
		{
			som_critical_count--;

			if (sigprocmask(SIG_SETMASK,som_critical_masks+som_critical_count,NULL))
			{
#ifdef SIGSYS
				raise(SIGSYS);
#else
				raise(SIGABRT);
#endif
			}
		}
		else
		{
#ifdef SIGSYS
			raise(SIGSYS);
#else
			raise(SIGABRT);
#endif
		}
	#endif
#endif
}

SOMEXTERN unsigned long SOMLINK somUniqueKey(somId id)
{
	SOM_IgnoreWarning(id);

	return 0;
}

SOMEXTERN void SOMLINK somBeginPersistentIds(void)
{
som_thread_globals_t *ev=SOMKERN_get_thread_globals(1);

	ev->persistent_ids++;
}

SOMEXTERN void SOMLINK somEndPersistentIds(void)
{
som_thread_globals_t *ev=SOMKERN_get_thread_globals(1);

	ev->persistent_ids--;
}

SOMEXTERN somId SOMLINK somCheckId (somId id)
{
	return id;
}

SOMEXTERN int SOMLINK somRegisterId(somId id)
{
	SOM_IgnoreWarning(id);

	return 0;
}

SOMEXTERN somId SOMLINK somIdFromString(char * aString)
{
	if (aString)
	{
		struct a
		{
			char * a;
			char n[1];
		};
		size_t len=strlen(aString);
		struct a *b=SOMCalloc(sizeof(*b)+len,1);
		memcpy(b->n,aString,len+1);
		b->a=b->n;

		return &b->a;
	}

	return 0;
}

SOMEXTERN unsigned long SOMLINK somTotalRegIds(void)
{
	return 0;
}

SOMEXTERN void SOMLINK somSetExpectedIds(unsigned long numIds)
{
	SOM_IgnoreWarning(numIds);
}

SOMEXTERN void SOMLINK somTest(int condition, int severity,const char * fileName,
                               int lineNum, char * msg)
{
	if (!condition)
	{
		somPrintf("somTest(severity=%d,fileName=%s,lineNum=%d,msg=%s)\n",
				severity,fileName,lineNum,msg);
	}
}

SOMEXTERN int SOMLINK somVprintf(const char * fmt, va_list ap)
{
#if defined(RHBOPT_SHARED_DATA) || defined(USE_THREADS)
som_thread_globals_t *tev=SOMKERN_get_thread_globals(0);
somTD_SOMOutCharRoutine *local_SOMOutCharRoutine=tev ? tev->somOutCharRoutine : SOMOutCharRoutine;
#endif

	if (local_SOMOutCharRoutine)
	{
		int i,j;
		char buf[4096];
		char *p;

/*		SOMKERN_guard*/

#ifdef HAVE_VSNPRINTF
		i=vsnprintf(buf,sizeof(buf),fmt,ap);
#else
	#ifdef HAVE_INT_VSPRINTF
		i=vsprintf(buf,fmt,ap);
	#else
		vsprintf(buf,fmt,ap);
		i=strlen(buf);
	#endif
#endif
		j=i;
		p=buf;

		while (i--)
		{
			local_SOMOutCharRoutine(*p);
			p++;
		}

		local_SOMOutCharRoutine(0);

/*		SOMKERN_unguard*/

		return j;
	}

	return 0;
}


/*
 * Outputs (via somPrintf) blanks to prefix a line at the indicated level
 */
SOMEXTERN void SOMLINK somPrefixLevel (long level)
{
#if defined(RHBOPT_SHARED_DATA) || defined(USE_THREADS)
som_thread_globals_t *tev=SOMKERN_get_thread_globals(0);
somTD_SOMOutCharRoutine *local_SOMOutCharRoutine=tev ? tev->somOutCharRoutine : SOMOutCharRoutine;
#endif

	if (local_SOMOutCharRoutine)
	{
		if (level)
		{
			unsigned long i;

			i=level << 1;

			while (i--)
			{
				local_SOMOutCharRoutine(' ');
			}
		}
	}
}

SOMEXTERN int SOMLINK somLPrintf (int level, char * fmt, ...)
{
	va_list va;
	int i;

	va_start(va,fmt);

	somPrefixLevel(level);

	i=somVprintf(fmt,va);

	va_end(va);

	return i;
}

#ifdef SOMKERN_END_THREAD
static void SOMKERN_end_thread(void *evv)
{
	if (evv)
	{
		som_thread_globals_t *ev=evv;

		somExceptionFree(&ev->ev);
#ifdef USE_THREADS
		SOMFree(ev);
#endif
	}
}
#endif


static struct somMethodTabStruct * SOMKERN_resolve_mtab(
		struct somMethodTabStruct *mtab)
{
	/*
	while (somClassInfoFromMtab(mtab)->substituted_mtab)
	{
		mtab=somClassInfoFromMtab(mtab)->substituted_mtab;
	}

	return mtab;
	*/

	/* 20070830 */

	return somClassInfoFromMtab(mtab)->parents.mtab;
}

static void SOMKERN_clear_somMethodTabPtr(somMethodTabPtr mtab)
{
	if (somClassInfoFromMtab(mtab)->sci) SOMKERN_clear_somStaticClassInfo(somClassInfoFromMtab(mtab)->sci);
}

static void SOMKERN_clear_somStaticClassInfo(somStaticClassInfo *sci)
{
	somStaticMethod_t *smt=sci->smt;
	int i=sci->numStaticMethods;

	if (sci->cds) 
	{
		sci->cds->classObject=NULL;
	}

	if (sci->ccds)
	{
		sci->ccds->parentMtab=NULL;
		sci->ccds->instanceDataToken=NULL;
	}

	while (i--)
	{
		*(smt->classData)=NULL;
		smt++;
	}
}


static somMToken SOMKERN_index_to_somMToken(somMethodTabPtr mtab,unsigned int i)
{
	somClassInfo info=somClassInfoFromMtab(mtab);
	struct somParentClassInfo *p=info->classes._buffer;
	int j=info->classes._length;

	while (j--)
	{
		if (i >= p->jump_table_offset)
		{
			unsigned int k=i-p->jump_table_offset;

			if (k < somClassInfoFromMtab(p->cls)->added_methods._length)
			{
				somMToken m=&somClassInfoFromMtab(p->cls)->added_methods._buffer[k];

				RHBOPT_ASSERT(i == SOMKERN_resolve_index(mtab,m))

				return m;
			}
		}

		p++;
	}

	return NULL;
}

SOMEXTERN SOMClassMgr SOMSTAR SOMLINK somMainProgram(void)
{
	SOMClassMgr SOMSTAR mgr=somEnvironmentNew();

#ifdef RHBOPT_SHARED_DATA
	SOMKERN_get_thread_globals(1);
#endif

	return mgr;
}

struct somDTSInfo
{
	somDestructInfo SOMObject_destructInfo[1];
	somDestructInfo SOMClass_destructInfo[2];
	somDestructInfo SOMClassMgr_destructInfo[2];

	somInitInfo SOMObject_initInfo[1];
	somInitInfo SOMClass_initInfo[2];
	somInitInfo SOMClassMgr_initInfo[2];

	somAssignInfo SOMObject_assignInfo[1];
	somAssignInfo SOMClass_assignInfo[2];
	somAssignInfo SOMClassMgr_assignInfo[2];
};

extern struct somDTSInfo __somdts;

static boolean somcm_is_kernel_class(somMethodTabPtr mtab)
{
	unsigned int i=SOM_Bootstrap_ccds_seq._length;
	
	while (i--)
	{
		/* 20070830 */
		RHBOPT_ASSERT(somMtabFromParentMtabs(SOM_Bootstrap_ccds_seq._buffer[i]->parentMtab)==
					SOM_Bootstrap_ccds_seq._buffer[i]->parentMtab->mtab);

		if (mtab==somMtabFromParentMtabs(SOM_Bootstrap_ccds_seq._buffer[i]->parentMtab))
		{
			return 1;
		}
	}

	return 0;
}

#include <rhbsomkd.c>
#include <somcm.c>
#include <somcls.c>
#include <somobj.c>

/* defines layout of a static class object */

#define _SOMObject_somClassInfoMethodTab	__somobj
#define _SOMClass_somClassInfoMethodTab		__somcls
#define _SOMClassMgr_somClassInfoMethodTab	__somcm

#define SOMKERN_classObject(x)		{  \
	(struct somMethodTabStruct *)(void *)&_SOMClass_somMTabs, \
	{(struct somClassInfoMethodTab *)(void *)&_##x##_somClassInfoMethodTab, \
	(somClassDataStructure *)(void *)&x##ClassData}};

#define SOMObject_somClassInfo    _SOMObject_somClassInfoMethodTab.classInfo
#define SOMClass_somClassInfo	  _SOMClass_somClassInfoMethodTab.classInfo
#define SOMClassMgr_somClassInfo  _SOMClassMgr_somClassInfoMethodTab.classInfo
#define _SOMObject_somMTabs		  _SOMObject_somClassInfoMethodTab.mtab
#define _SOMClass_somMTabs		  _SOMClass_somClassInfoMethodTab.mtab
#define _SOMClassMgr_somMTabs	  _SOMClassMgr_somClassInfoMethodTab.mtab

#define SOMKERN_initCtrl(x)		{x##_initMask,__somdts.x##_initInfo,sizeof(somInitInfo),NULL}
#define SOMKERN_destructCtrl(x) {x##_destructMask,__somdts.x##_destructInfo,sizeof(somDestructInfo),NULL}
#define SOMKERN_assignCtrl(x)	{x##_assignMask,__somdts.x##_assignInfo,sizeof(somAssignInfo),NULL}

static boolean SOMObject_initMask[]={0};
static boolean SOMClass_initMask[]={0,0};
static boolean SOMClassMgr_initMask[]={0,0};

static boolean SOMObject_destructMask[]={0};
static boolean SOMClass_destructMask[]={0,0};
static boolean SOMClassMgr_destructMask[]={0,0};

static boolean SOMObject_assignMask[]={0};
static boolean SOMClass_assignMask[]={0,0};
static boolean SOMClassMgr_assignMask[]={0,0};


static void SOMLINK SOMObject_somRenewNoInitNoZeroThunk(void *pv)
{
	((SOMObject SOMSTAR)pv)->mtab=SOMObjectCClassData.parentMtab->mtab;
}

static void SOMLINK SOMClass_somRenewNoInitNoZeroThunk(void *pv)
{
	((SOMObject SOMSTAR)pv)->mtab=SOMClassCClassData.parentMtab->mtab;
}

static void SOMLINK SOMClassMgr_somRenewNoInitNoZeroThunk(void *pv)
{
	((SOMObject SOMSTAR)pv)->mtab=SOMClassMgrCClassData.parentMtab->mtab;
}

#include <somkernp.kih>

#define  SOMObject_dataOffset		0
#define  SOMClass_dataOffset		(long)(size_t)&(((struct SOMClass_SOMAny *)NULL)->SOMClass_data)
#define  SOMClassMgr_dataOffset		(long)(size_t)&(((struct SOMClassMgr_SOMAny *)NULL)->SOMClassMgr_data)

struct somDTSInfo __somdts={
	/* somDestructInfo SOMObject */
	{
		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDestruct,
		SOMObject_dataOffset,
		NULL}
	},
	/* somDestructInfo SOMClass */
	{
		{(SOMClass SOMSTAR)(void *)&SOMClass_classObject,
		(somMethodPtr)somcls_somDestruct,
		SOMClass_dataOffset,
		(somMethodPtr)somcls_somUninit},

		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDestruct,
		SOMObject_dataOffset,
		NULL}
	},
	/* somDestructInfo SOMClassMgr */
	{
		{(SOMClass SOMSTAR)(void *)&SOMClassMgr_classObject,
		(somMethodPtr)somcm_somDestruct,
		SOMClassMgr_dataOffset,
		(somMethodPtr)somcm_somUninit},

		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDestruct,
		SOMObject_dataOffset,
		NULL}
	},
	/* somInitInfo SOMObject */
	{
		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDefaultInit,
		(somMethodPtr)somobj_somDefaultCopyInit,
		(somMethodPtr)somobj_somDefaultConstCopyInit,
		(somMethodPtr)somobj_somDefaultVCopyInit,
		SOMObject_dataOffset,
		NULL}
	},
	/* somInitInfo SOMClass */
	{
		{(SOMClass SOMSTAR)(void *)&SOMClass_classObject,
		(somMethodPtr)somcls_somDefaultInit,
		(somMethodPtr)somobj_somDefaultCopyInit,
		(somMethodPtr)somobj_somDefaultConstCopyInit,
		(somMethodPtr)somobj_somDefaultVCopyInit,
		SOMClass_dataOffset,
		NULL},

		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDefaultInit,
		(somMethodPtr)somobj_somDefaultCopyInit,
		(somMethodPtr)somobj_somDefaultConstCopyInit,
		(somMethodPtr)somobj_somDefaultVCopyInit,
		SOMObject_dataOffset,
		NULL}
	},
	/* somInitInfo SOMClassMgr */
	{
		{(SOMClass SOMSTAR)(void *)&SOMClassMgr_classObject,
		(somMethodPtr)somcm_somDefaultInit,
		(somMethodPtr)somobj_somDefaultCopyInit,
		(somMethodPtr)somobj_somDefaultConstCopyInit,
		(somMethodPtr)somobj_somDefaultVCopyInit,
		SOMClassMgr_dataOffset,
		NULL},

		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDefaultInit,
		(somMethodPtr)somobj_somDefaultCopyInit,
		(somMethodPtr)somobj_somDefaultConstCopyInit,
		(somMethodPtr)somobj_somDefaultVCopyInit,
		SOMObject_dataOffset,
		NULL}
	},
	/* somAssignInfo SOMObject */
	{
		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDefaultAssign,
		(somMethodPtr)somobj_somDefaultConstAssign,
		(somMethodPtr)somobj_somDefaultAssign,
		NULL,
		NULL,
		SOMObject_dataOffset}
	},
	/* somAssignInfo SOMClass */
	{
		{(SOMClass SOMSTAR)(void *)&SOMClass_classObject,
		(somMethodPtr)somobj_somDefaultAssign,
		(somMethodPtr)somobj_somDefaultConstAssign,
		(somMethodPtr)somobj_somDefaultAssign,
		NULL,
		NULL,
		SOMClass_dataOffset},

		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDefaultAssign,
		(somMethodPtr)somobj_somDefaultConstAssign,
		(somMethodPtr)somobj_somDefaultAssign,
		NULL,
		NULL,
		SOMObject_dataOffset}
	},
	/* somAssignInfo SOMClassMgr */
	{
		{(SOMClass SOMSTAR)(void *)&SOMClassMgr_classObject,
		(somMethodPtr)somobj_somDefaultAssign,
		(somMethodPtr)somobj_somDefaultConstAssign,
		(somMethodPtr)somobj_somDefaultAssign,
		NULL,
		NULL,
		SOMClassMgr_dataOffset},

		{(SOMClass SOMSTAR)(void *)&SOMObject_classObject,
		(somMethodPtr)somobj_somDefaultAssign,
		(somMethodPtr)somobj_somDefaultConstAssign,
		(somMethodPtr)somobj_somDefaultAssign,
		NULL,
		NULL,
		SOMObject_dataOffset}
	}
};

static boolean somcm_is_leaf(SOMClassMgrData *somThis,somMethodTabPtr mtab)
{
	/* see if it is a static class... */


	int i;

	if (somcm_is_kernel_class(mtab)) 
	{
		return 0;
	}
		
	i=RHBCDR_kds_locked_length(&somThis->classList);

	while (i--)
	{
		RHBSOMUT_KeyData *kd=RHBCDR_kds_locked_get(&somThis->classList,i);
		somMethodTabPtr t=kd->data._value;
		if (t!=mtab)
		{
			/* don't check against self */

			unsigned int j=somClassInfoFromMtab(t)->classes._length;
			struct somParentClassInfo *par=somClassInfoFromMtab(t)->classes._buffer;

			while (j--)
			{
				if (par->cls==mtab)
				{
/*					somPrintf("-- used by %s\n",t->className);*/
					return 0;
				}

				par++;
			}
		}

		if (t->classObject->mtab==mtab) return 0;
	}

	return 1;
}

static void somcm_teardown(SOMClassMgr SOMSTAR somSelf)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	unsigned int i;
	
	SOMClassMgr__set_somInterfaceRepository(somSelf,NULL);

	i=RHBCDR_kds_locked_length(&somThis->classList);

	while (i--)
	{
		RHBSOMUT_KeyData *kd=RHBCDR_kds_locked_get(&somThis->classList,i);
		somMethodTabPtr mtab=kd->data._value;

/*		somPrintf("looking at %s\n",mtab->className);*/

		if (somcm_is_leaf(somThis,mtab))
		{
/*			somPrintf("unregistering %s\n",mtab->className);*/

			SOMClassMgr_somUnregisterClass(somSelf,mtab->classObject);

			i=RHBCDR_kds_locked_length(&somThis->classList);
		}
	}
}

#ifdef SOM_RESOLVE_DATA
SOMEXTERN somTD_SOMOutCharRoutine ** SOMLINK resolve_SOMOutCharRoutine(void)
{
#ifdef RHBOPT_SHARED_DATA
	som_thread_globals_t *tev=SOMKERN_get_thread_globals(1);
	return tev ? &tev->somOutCharRoutine : NULL;
#else
	return &SOMOutCharRoutine;
#endif
}
#endif /* SOM_RESOLVE_DATA */

#ifdef somStartCriticalSection
#undef somStartCriticalSection
SOMEXTERN void SOMLINK somStartCriticalSection(void)
{
	somStartCriticalSectionDebug(__FILE__,__LINE__);
}
#endif

static struct SOMClassMgr_SOMAny SOMClassMgrObjectData=
{
	(somMethodTabPtr)(void *)&_SOMClassMgr_somMTabs,
	{
		RHBSOMUT_KeyDataSetInit,
		NULL,
		NULL,
		(SOMClassMgr SOMSTAR)(void *)&SOMClassMgrObjectData
	}
};

#if defined(USE_THREADS) && !defined(USE_PTHREADS)
#define SOMClassMgrObject_CRITICAL_SECTION		\
		&SOMClassMgrObjectData.SOMClassMgr_data.classList.guardian.crit_sect
#endif

static struct
{
	/* this holds a linked list of classMgr objects */
	SOMClassMgrData *first;
	SOMClassMgrData *last;
} somClassMgrList={
		&SOMClassMgrObjectData.SOMClassMgr_data,
		&SOMClassMgrObjectData.SOMClassMgr_data};

SOMClassMgr SOMSTAR SOMClassMgrObject=(SOMClassMgr SOMSTAR)(void *)&SOMClassMgrObjectData;

SOM_Scope void SOMLINK somcm_somDefaultInit(SOMClassMgr SOMSTAR somSelf,
											 somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	SOMClassMgrData *somThis;
	somBooleanVector myMask;

	SOMClassMgr_BeginInitializer_somDefaultInit

	RHBOPT_ASSERT(!(myMask[0] & 1));

	SOMClassMgr_Init_SOMObject_somDefaultInit(somSelf,ctrl);

	RHBCDR_kds_init(&somThis->classList);

	somStartCriticalSection();

	somThis->next=NULL;
	somThis->prev=somClassMgrList.last;
	somClassMgrList.last->next=somThis;
	somClassMgrList.last=somThis;

	somEndCriticalSection();
}

SOM_Scope void SOMLINK somcm_somDestruct(SOMClassMgr SOMSTAR somSelf,
										 boolean doFree,
										 somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	SOMClassMgrData *somThis;
	somBooleanVector myMask;

	SOMClassMgr_BeginDestructor

	RHBOPT_ASSERT(!(myMask[0] & 1));

/*
	somPrintf("somcm_somDestruct(%p,%p,%p)\n",
			somSelf,
			somThis,
			SOMClassMgrGetData(somSelf));
	somPrintf("should be (%p,%p)\n",
			&SOMClassMgrObjectData,
			&SOMClassMgrObjectData.SOMClassMgr_data);
*/
	somStartCriticalSection();

	while (somThis->classList.dataset._length)
	{
		RHBSOMUT_KeyData *kdp=somThis->classList.dataset._buffer[
			--(somThis->classList.dataset._length)];
		kdp->count--;
	}

	somEndCriticalSection();

/*	somcm_somUninit(somSelf); */

	if (somSelf==(void *)&SOMClassMgrObjectData)
	{
		_IDL_SEQUENCE_RHBSOMUT_KeyDataPtr seq={0,0,NULL};
		void *pv=somThis->classList.dataset._buffer;
		somThis->classList.dataset=seq;
		if (pv) SOMFree(pv);
		doFree=0; /* don't free if it's the static one */
	}
	else
	{
		/* if destroying this and we are the current class mgr
		   then swizzle back the pointers */

		if (somSelf==SOMClassMgrObject)
		{
			SOMClassMgrObject=(void *)&SOMClassMgrObjectData;
		}

		if (somThis->next)
		{
			RHBOPT_ASSERT(somThis->next->prev==somThis);

			somThis->next->prev=somThis->prev;
		}
		else
		{
			RHBOPT_ASSERT(somClassMgrList.last==somThis);

			somClassMgrList.last=somThis->prev;
		}

		if (somThis->prev)
		{
			RHBOPT_ASSERT(somThis->prev->next==somThis);

			somThis->prev->next=somThis->next;
		}
		else
		{
			RHBOPT_ASSERT(somClassMgrList.first==somThis);

			somClassMgrList.first=somThis->next;
		}

		SOMClassList_destroy(somThis);
	}

	SOMClassMgr_EndDestructor
}


boolean SOMLINK somIsObj(somToken obj)
{
	SOMObject SOMSTAR o=(SOMObject SOMSTAR)obj;

	if (o)
	{
		if (o->mtab)
		{
			somMethodTabPtr mtab=o->mtab;

#define CHECK_MTAB(x)  if (mtab==(void *)&_##x##_somClassInfoMethodTab.mtab) return 1;

			CHECK_MTAB(SOMObject)
			CHECK_MTAB(SOMClass)
			CHECK_MTAB(SOMClassMgr)

#undef CHECK_MTAB

			/* there is a window of opportunity to fail
				during SOMClassMgr_somDestruct
			*/

			{
				somClassInfo info=somClassInfoFromMtab(o->mtab);
				SOMClassMgrData *somClassMgr=somClassMgrList.first;

				while (somClassMgr)
				{
					if (RHBCDR_kds_contains(
						&(somClassMgr->classList),
						&(info->keyed_data)))
					{
						return 1;
					}

					somClassMgr=somClassMgr->next;
				}
			}
		}
	}

	return 0;
}

/*******************************************************************
 * free the thread local data on demand
 */

SOMEXTERN void SOMLINK somFreeThreadData(void)
{
	som_thread_globals_t *ev=SOMKERN_get_thread_globals(0);

	if (ev)
	{
#ifdef SOMKERN_END_THREAD
		SOMKERN_end_thread(ev);
#else
		somExceptionFree(&ev->ev);
#endif

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_setspecific(som_globals.tls_key,NULL);
	#else
		TlsSetValue(som_globals.tls_key,NULL);
	#endif
#endif
	}
}

#ifdef _WIN32
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
    {
    case DLL_PROCESS_ATTACH:
#ifdef SOMClassMgrObject_CRITICAL_SECTION
		InitializeCriticalSection(SOMClassMgrObject_CRITICAL_SECTION);
#endif
#if defined(_WIN32) && !defined(_WIN32S)
		som_tidMain=GetCurrentThreadId();
#endif
		rhbsomkd_init();
		SOM_dll_init();
		return 1;
    case DLL_PROCESS_DETACH:
		SOM_dll_term();
		rhbsomkd_term();
#ifdef SOMClassMgrObject_CRITICAL_SECTION
		DeleteCriticalSection(SOMClassMgrObject_CRITICAL_SECTION);
#endif
       	return 1;
	case DLL_THREAD_DETACH:
#if !defined(USE_PTHREADS)
		somFreeThreadData();
#endif
#if defined(_WIN32) && !defined(_WIN32S)
		if (GetCurrentThreadId()==som_tidMain)
		{
			som_tidMain=0;
		}
#endif
		return 1;
	}
    
	SOM_IgnoreWarning(hInst);
	SOM_IgnoreWarning(extra);

   	return 1;
}
#endif

SOMEXTERN SOMClassMgr SOMSTAR SOMLINK somEnvironmentNew(void)
{
#ifdef _DEBUG
	{
		somParentMtabStructPtr p=(void *)SOMClassCClassData.parentMtab;
	
		RHBOPT_ASSERT(SOMClassClassData.classObject);
		RHBOPT_ASSERT(p->classObject==SOMClassClassData.classObject);
	}
	{
		somParentMtabStructPtr p=(void *)SOMObjectCClassData.parentMtab;

		RHBOPT_ASSERT(SOMObjectClassData.classObject);
		RHBOPT_ASSERT(p->classObject==SOMObjectClassData.classObject);
	}
	{
		somParentMtabStructPtr p=(void *)SOMClassMgrCClassData.parentMtab;
	
		RHBOPT_ASSERT(SOMClassMgrClassData.classObject);
		RHBOPT_ASSERT(p->classObject==SOMClassMgrClassData.classObject);
	}
#endif

	SOM_THREAD_INIT_ONCE

	return SOMClassMgrObject;
}

#ifdef SOMKERN_DLL_INIT
static void SOM_dll_init(void)
{
	if (!som_globals.dll_alive)
	{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_KEY_CREATE
			pthread_key_create(&som_globals.tls_key,SOMKERN_end_thread);
		#else
			pthread_keycreate(&som_globals.tls_key,SOMKERN_end_thread);
		#endif
		#ifdef RHBMUTEX_INIT_DATA
		#else
			RHBMUTEX_INIT(&som_global_mutex);
			RHBCDR_kds_init(&SOMClassMgrObjectData.SOMClassMgr_data.classList);
		#endif
	#else
			som_globals.tls_key=TlsAlloc();
			InitializeCriticalSection(&som_global_mutex);
	#endif
#endif
	}
	som_globals.dll_alive++;
}
#endif

#ifdef SOMKERN_DLL_TERM
static void SOM_dll_term(void)
{
	if (som_globals.dll_alive)
	{
		if (!--som_globals.dll_alive)
		{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#ifdef HAVE_PTHREAD_KEY_DELETE
			pthread_key_delete(som_globals.tls_key);
		#endif
	#else
			DeleteCriticalSection(&som_global_mutex);
			TlsFree(som_globals.tls_key);
	#endif
#endif
		}
	}
}
#endif
