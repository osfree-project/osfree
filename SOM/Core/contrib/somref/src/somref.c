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

#define SOM_Module_somref_Source
#define SOMRefObject_Class_Source

#ifdef _WIN32
	#include <windows.h>
#endif

#include <rhbsomex.h>

#ifdef SOMREF_DEBUG
typedef struct SOMRefObjectPrivateData
{
	long fCount;
#ifdef _DEBUG
	char fClassName[256];
#endif
} SOMRefObjectPrivateData;
#define fRefCount		fData.fCount
#else
typedef long SOMRefObjectPrivateData;
#define fRefCount		fData
#endif

#include "somref.ih"

#ifdef SOMRefObject_parent_SOMObject_somFree

#if (SOMObject_MajorVersion > 1) || (SOMObject_MinorVersion >= 7)
#	ifndef SOMRefObject_duplicate
		#error did not pickup hidden override SOMObject::duplicate
#	endif

#	ifndef SOMRefObject_parent_SOMObject_duplicate
		#error did not pickup hidden override SOMObject::duplicate
#	endif
#endif

/* #define ATOMICADDTOMEMORY(mem,x)  (*(mem)+=(x))*/

#ifndef ATOMIC_INCREMENT
#	ifdef USE_THREADS
#		ifdef USE_PTHREADS
#			ifdef RHBOPT_PTHREAD_MUTEX_INITIALIZER
			static pthread_mutex_t somref_mutex=RHBOPT_PTHREAD_MUTEX_INITIALIZER;
#			else
			static pthread_mutex_t somref_mutex;
			static pthread_once_t somref_once=PTHREAD_ONCE_INIT;
			static void somref_once_r(void)
			{
				pthread_mutex_init(&somref_mutex,RHBOPT_pthread_mutexattr_default);
			}
#			endif
			static long ATOMIC_INCREMENT(long *mem)
			{
				register long res;
				pthread_mutex_lock(&somref_mutex);
				res=(++(*(mem)));
				pthread_mutex_unlock(&somref_mutex);
				return res;
			}
			static long ATOMIC_DECREMENT(long *mem)
			{
				register long res;
				pthread_mutex_lock(&somref_mutex);
				res=(--(*(mem)));
				pthread_mutex_unlock(&somref_mutex);
				return res;
			}
#		else
#			define ATOMIC_INCREMENT(x)		InterlockedIncrement(x)
#			define ATOMIC_DECREMENT(x)		InterlockedDecrement(x)
#		endif
#	else
#		define ATOMIC_INCREMENT(mem)  (++(*(mem)))
#		define ATOMIC_DECREMENT(mem)  (--(*(mem)))
#	endif
#endif

SOM_Scope SOMObject SOMSTAR SOMLINK somro_somDuplicateReference(
		SOMRefObject SOMSTAR somSelf)
{
    SOMRefObjectData *somThis = SOMRefObjectGetData(somSelf);

	RHBOPT_ASSERT(somThis->fRefCount>=0);

	ATOMIC_INCREMENT( & somThis->fRefCount);

    return somSelf;
}

SOM_Scope boolean  SOMLINK somro_somCompareReference(
		SOMRefObject SOMSTAR somSelf, 
        SOMObject SOMSTAR anObject)
{
	return ((SOMObject SOMSTAR) somSelf) == anObject; 
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somro_somRelease(
		SOMRefObject SOMSTAR somSelf)
{
	SOMRefObjectData *somThis = SOMRefObjectGetData ( somSelf );

	RHBOPT_ASSERT(somSelf);
	RHBOPT_ASSERT(somThis);
	RHBOPT_ASSERT(somThis->fRefCount>=0);

	if (ATOMIC_DECREMENT(&somThis->fRefCount) < 0) 
	{ 
		if (SOMRefObject_somCanDelete (somSelf) ) 
		{
/*			SOMRefObject_parent_SOMObject_somFree ( somSelf ); */

#ifdef SOMRefObject_somDestruct
			SOMRefObject_somDestruct(somSelf,1,NULL);
#else
			SOMRefObject_parent_SOMObject_somFree (somSelf); 
#endif

			return NULL; 
		}
	} 

	return somSelf;
}

SOM_Scope boolean  SOMLINK somro_somCanDelete(
	SOMRefObject SOMSTAR somSelf)
{
	return 1; 
}

SOM_Scope void  SOMLINK somro_somFree(
	SOMRefObject SOMSTAR somSelf)
{
	SOMRefObjectData *somThis = SOMRefObjectGetData ( somSelf ); 

	RHBOPT_ASSERT(somSelf);
	RHBOPT_ASSERT(somThis);
	RHBOPT_ASSERT(somThis->fRefCount>=0);

	if ( ATOMIC_DECREMENT(&somThis->fRefCount) < 0 ) 
	{ 
		if (SOMRefObject_somCanDelete(somSelf) ) 
		{
#ifdef SOMRefObject_somDestruct
			SOMRefObject_somDestruct(somSelf,1,NULL);
#else
			SOMRefObject_parent_SOMObject_somFree (somSelf); 
#endif
		}
	}
}

	#ifdef SOMREF_DEBUG
		SOM_Scope void SOMLINK somro_somInit(
				SOMRefObject SOMSTAR somSelf)
		{
			SOMRefObjectData *somThis=SOMRefObjectGetData ( somSelf ); 

			somThis->fRefCount=0;

		#ifdef SOMREF_DEBUG
			strncpy(somThis->fData.fClassName,
					somSelf->mtab->className,
					sizeof(somThis->fData.fClassName));
		#endif
			SOMRefObject_parent_SOMObject_somInit(somSelf);
		}
	#endif
#endif

SOMInitModule_begin(somref)

   SOMInitModule_new(SOMRefObject);

SOMInitModule_end

#ifdef _PLATFORM_WIN32_
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
#ifdef SOMObject_somRelease
				return 0;
#else
				return DisableThreadLibraryCalls(hInst);
#endif
	        case DLL_PROCESS_DETACH:
            		return 1;
	}

   	return 1;
}
#endif

#ifdef SOMRefObject_release
SOM_Scope void SOMLINK somro_release(
			 SOMRefObject SOMSTAR somSelf,
			 Environment *ev)
{
	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(NO_EXCEPTION==ev->_major);

	if (NO_EXCEPTION==ev->_major)
	{
		SOMRefObject_somRelease(somSelf);
	}
}
#endif

#ifdef SOMRefObject_duplicate
SOM_Scope SOMObject SOMSTAR SOMLINK somro_duplicate(
			 SOMRefObject SOMSTAR somSelf,
			 Environment *ev)
{
	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(NO_EXCEPTION==ev->_major);

	if (NO_EXCEPTION==ev->_major)
	{
		return SOMRefObject_somDuplicateReference(somSelf);
	}

	return NULL;
}
#endif

SOM_Scope void SOMLINK somro_somDefaultInit(
		SOMRefObject SOMSTAR somSelf,
		somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SOMRefObjectData *somThis;

	SOMRefObject_BeginInitializer_somDefaultInit

	RHBOPT_unused(somThis);
	RHBOPT_ASSERT(!(myMask[0]&1));
	RHBOPT_ASSERT(somThis==SOMRefObjectGetData(somSelf));

	SOMRefObject_Init_SOMObject_somDefaultInit(somSelf,ctrl);
}

SOM_Scope void SOMLINK somro_somDestruct(
		SOMRefObject SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SOMRefObjectData *somThis;

	SOMRefObject_BeginDestructor;

	RHBOPT_unused(somThis);

	RHBOPT_ASSERT(!(myMask[0] & 1));

	RHBOPT_ASSERT(somThis==SOMRefObjectGetData(somSelf));

	SOMRefObject_EndDestructor;
}

static void SOMLINK somro_clsInit(SOMClass SOMSTAR somSelf)
{
#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#		ifdef RHBOPT_PTHREAD_MUTEX_INITIALIZER
#		else
			pthread_once(&somref_once,somref_once_r);
#		endif
#	endif
#endif
}


SOMEXTERN SOMClassMgr SOMSTAR SOMLINK somGetClassManagerReference(void)
{
	SOMClassMgr mgr=somEnvironmentNew();

	if (mgr)
	{
#ifdef SOMClassMgr_duplicate
		Environment ev;

		SOM_EnvironmentInit(&ev);

		mgr=SOMClassMgr_duplicate(mgr,&ev);

		SOM_UninitEnvironment(&ev);
#else
		if (SOMClassMgr_somIsA(mgr,_SOMRefObject))
		{
			mgr=SOMRefObject_somDuplicateReference(mgr);
		}
#endif
	}

	return mgr;
}

SOMEXTERN void SOMLINK somReleaseObjectReference(SOMObject SOMSTAR obj)
{
#ifdef SOMObject_release
	Environment ev;
	SOM_InitEnvironment(&ev);
	SOMObject_release(obj,&ev);
	SOM_UninitEnvironment(&ev);
#else
	SOM_Assert(SOMObject_somIsA(obj,_SOMRefObject),1);
	SOMRefObject_somRelease(obj);
#endif
}

SOMEXTERN void SOMLINK somReleaseClassReference(SOMClass SOMSTAR cls)
{
#ifdef SOMClass_release
	Environment ev;
	SOM_InitEnvironment(&ev);
	SOMClass_release(cls,&ev);
	SOM_UninitEnvironment(&ev);
#else
	SOM_Assert(SOMClassMgr_somIsA(cls,_SOMClass),1);
#endif
}

SOMEXTERN void SOMLINK somReleaseClassManagerReference(SOMClass SOMSTAR clsMgr)
{
#ifdef SOMClassMgr_release
	Environment ev;
	SOM_InitEnvironment(&ev);
	SOMClassMgr_release(clsMgr,&ev);
	SOM_UninitEnvironment(&ev);
#else
	SOM_Assert(SOMClassMgr_somIsA(clsMgr,_SOMClassMgr),1);
#endif
}

SOMEXTERN corbastring SOMLINK somMakeStringFromId(somId id)
{
	corbastring q=NULL;
	
	if (id)
	{
		const char *p=somStringFromId(id);

		if (p)
		{
			int i=(int)strlen(p)+1;
			q=SOMMalloc(i);
			if (q)
			{
				memcpy(q,p,i);
			}
		}
	}

	return q;
}

