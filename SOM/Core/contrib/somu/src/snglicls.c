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

#ifndef SOM_Module_snglicls_Source
#define SOM_Module_snglicls_Source
#endif
#define SOMMSingleInstance_Class_Source

#include <rhbsomex.h>

#ifndef USE_APPLE_SOM
	#include <somref.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
	#include <Processes.h>
#endif

typedef struct SOMMSingleInstancePrivateData
{
	SOMObject SOMSTAR singleObject;
#ifdef RHBOPT_SHARED_DATA
	#ifdef _WIN32
		unsigned long tid;
	#else
		ProcessSerialNumber psn;
	#endif
	struct SOMMSingleInstancePrivateData *next;
#endif
} SOMMSingleInstancePrivateData;

typedef struct
{
#ifdef RHBOPT_SHARED_DATA
	SOMMSingleInstancePrivateData *list;
#else
	SOMMSingleInstancePrivateData priv;
#endif
} snglicls_private;

#include "snglicls.ih"

#include <stdio.h>
#include <string.h>

static SOMMSingleInstancePrivateData *SOMMSingleInstancePrivateGetData(SOMMSingleInstanceData *somThis,boolean fCreate)
{
#ifdef RHBOPT_SHARED_DATA
	SOMMSingleInstancePrivateData *priv=NULL;
	#ifdef _WIN32
		unsigned long tid=GetCurrentThreadId();
	#else
		ProcessSerialNumber psn;
		GetCurrentProcess(&psn);
	#endif

	priv=somThis->private_data.list;

	while (priv)
	{
#ifdef _WIN32
		if (priv->tid==tid)
		{
			break;
		}
#else
		Boolean b=0;
		if (!SameProcess(&priv->psn,&psn,&b))
		{
			if (b)
			{
				break;
			}
		}
#endif

		priv=priv->next;
	}

	if (fCreate && !priv)
	{
		priv=SOMMalloc(sizeof(*priv));
		if (priv)
		{
#ifdef _WIN32
			priv->tid=tid;
#else
			priv->psn=psn;
#endif
			priv->singleObject=NULL;
			priv->next=somThis->private_data.list;
			somThis->private_data.list=priv;
		}
	}

	return priv;
#else
	return &somThis->private_data.priv;
#endif
}

static void SOMMSingleInstancePrivateFree(SOMMSingleInstanceData *somThis,SOMMSingleInstancePrivateData *somPrivate)
{
	if (somThis && somPrivate)
	{
#ifdef RHBOPT_SHARED_DATA
		if (somThis->private_data.list==somPrivate)
		{
			somThis->private_data.list=somPrivate->next;
		}
		else
		{
			SOMMSingleInstancePrivateData *p=somThis->private_data.list;

			while (p)
			{
				if (p->next==somPrivate)
				{
					p->next=somPrivate->next;
					break;
				}
				else
				{
					p=p->next;
				}
			}
		}

		SOMFree(somPrivate);
#endif
	}
}

#define SOMMSingleInstance_unused(x)    if (x) { ; }

SOM_Scope SOMObject SOMSTAR  SOMLINK 
	snglicls_sommGetSingleInstance(
		SOMMSingleInstance SOMSTAR somSelf, 
			Environment *ev)
{
    SOMMSingleInstanceData *somThis = SOMMSingleInstanceGetData(somSelf);
	SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,1);
	SOMMSingleInstance_unused(ev)

	if (somPrivate->singleObject) 
	{
#ifdef SOMObject_somDuplicateReference
		return SOMObject_somDuplicateReference(somPrivate->singleObject);
#else
		SOMClass SOMSTAR cls=somNewClassReference(SOMRefObject);

		if (cls)
		{
			if (SOMObject_somIsA(somPrivate->singleObject,
					cls))
			{
				somReleaseClassReference(cls);

				return SOMRefObject_somDuplicateReference(somPrivate->singleObject);
			}
		}

		somReleaseClassReference(cls);


		/* could call SOMObject_duplicate */

		return somPrivate->singleObject;
#endif
	}

    somPrivate->singleObject=SOMClass_somNew(somSelf);

	return somPrivate->singleObject;
}

SOM_Scope void  SOMLINK snglicls_sommFreeSingleInstance(
		SOMMSingleInstance SOMSTAR somSelf, 
		Environment *ev)
{
	SOMMSingleInstanceData *somThis=SOMMSingleInstanceGetData(somSelf);
	SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,0);
	if (somPrivate)
	{
		SOMObject SOMSTAR o=somPrivate->singleObject;
		somPrivate->singleObject=NULL;
 
		SOMMSingleInstancePrivateFree(somThis,somPrivate);

		SOMMSingleInstance_unused(ev)

		if (o) 
		{
			somReleaseObjectReference(o);
		}
	}
}

struct snglicls_somNew
{
	SOMObject SOMSTAR ret;
	somToken mutex;
};

RHBOPT_cleanup_begin(snglicls_somNew_cleanup,pv)

	struct snglicls_somNew *data=pv;
#ifdef USE_THREADS
	if (data->mutex) SOMReleaseMutexSem(data->mutex);
#endif
	if (data->ret) SOMObject_somFree(data->ret);

RHBOPT_cleanup_end

SOM_Scope SOMObject SOMSTAR  SOMLINK snglicls_somNew(
		SOMMSingleInstance SOMSTAR somSelf)
{
    SOMMSingleInstanceData *somThis = SOMMSingleInstanceGetData(somSelf);
	SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,1);
	struct snglicls_somNew data={NULL,NULL};
	SOMObject SOMSTAR RHBOPT_volatile return_obj=NULL;

	RHBOPT_cleanup_push(snglicls_somNew_cleanup,&data);

#ifdef USE_THREADS
	RHBOPT_ASSERT(somThis->mutex);
	SOMRequestMutexSem(somThis->mutex);
	data.mutex=somThis->mutex;
#endif

	data.ret=somPrivate->singleObject;

	if (data.ret) 
	{
#ifdef SOMObject_somDuplicateReference
		data.ret=SOMObject_somDuplicateReference(data.ret);
#else
		SOMClass SOMSTAR cls=somNewClassReference(SOMRefObject);

		if (SOMObject_somIsA(data.ret,cls))
		{
			data.ret=SOMRefObject_somDuplicateReference(data.ret);
		}
		else
		{
			static char *str_duplicate="duplicate";
			somToken tok=NULL;
			Environment ev;
			SOM_InitEnvironment(&ev);
			somva_SOMObject_somDispatch(data.ret,&tok,&str_duplicate,data.ret,&ev);
			SOM_UninitEnvironment(&ev);
		}

		somReleaseClassReference(cls);
#endif
	}
	else
	{
		data.ret=SOMMSingleInstance_parent_SOMClass_somNew(somSelf);
		if (data.ret)
		{
		    somPrivate->singleObject=data.ret;
		}
	}

	return_obj=data.ret;
	data.ret=NULL;

	RHBOPT_cleanup_pop();

	return return_obj;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK snglicls_somNewNoInit(
			SOMMSingleInstance SOMSTAR somSelf)
{
    SOMMSingleInstanceData *somThis = SOMMSingleInstanceGetData(somSelf);
	SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,1);

	if (somPrivate)
	{
		if (somPrivate->singleObject) return somPrivate->singleObject;

		somPrivate->singleObject=SOMMSingleInstance_parent_SOMClass_somNewNoInit(somSelf);

		return somPrivate->singleObject;
	}

	return NULL;
}

SOM_Scope void  SOMLINK snglicls_somInit(
	SOMMSingleInstance SOMSTAR somSelf)
{
    SOMMSingleInstanceData *somThis = SOMMSingleInstanceGetData(somSelf);
	SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,0);

#ifdef USE_THREADS
	SOMCreateMutexSem(&somThis->mutex);
#endif

	if (somPrivate)
	{
		somPrivate->singleObject=NULL;
	}

    SOMMSingleInstance_parent_SOMClass_somInit(somSelf);
}

SOM_Scope void  SOMLINK snglicls_somUninit(
	SOMMSingleInstance SOMSTAR somSelf)
{
    SOMMSingleInstanceData *somThis = SOMMSingleInstanceGetData(somSelf);

#ifdef RHBOPT_SHARED_DATA
	SOMMSingleInstancePrivateData *somPrivate=somThis->private_data.list;
	while (somPrivate)
	{
		SOMMSingleInstancePrivateData *next=somPrivate->next;
		SOMObject SOMSTAR o=somPrivate->singleObject;
		somPrivate->singleObject=NULL;

		if (o) 
		{
			somPrintf("SOMMSingleInstance(%s,%p) left over\n",
				SOMClass_somGetName(somSelf),o);
		}

#ifdef USE_THREADS
		SOMDestroyMutexSem(somPrivate->mutex);
#endif

		SOMFree(somPrivate);
		
		somPrivate=next;
	}
#else
	SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,0);

	if (somPrivate)
	{
		SOMObject SOMSTAR o=somPrivate->singleObject;
		somPrivate->singleObject=NULL;

		if (o) 
		{
			somPrintf("SOMMSingleInstance(%s,%p) left over\n",
				SOMClass_somGetName(somSelf),o);

			RHBOPT_ASSERT(!o);

	/*		SOMObject_somUninit(o);
			SOMClass_somDeallocate(somSelf,(char *)o);*/
		}

	/*	somPrintf("%s being freed\n",SOMClass_somGetName(somSelf));*/
	}
#endif

#ifdef USE_THREADS
	if (somThis->mutex)
	{
		SOMDestroyMutexSem(somThis->mutex);
		somThis->mutex=NULL;
	}
#endif

    SOMMSingleInstance_parent_SOMClass_somUninit(somSelf);
}

SOM_Scope void SOMLINK snglicls_somClassReady(
	SOMMSingleInstance SOMSTAR somSelf)
{
#ifdef NO_extra_bits
	SOMMSingleInstanceData *somThis=SOMMSingleInstanceGetData(somSelf);

	if (!somThis->old_somUninitPtr)
	{
		somMethodPtr newPtr;
		_IDL_SEQUENCE_SOMClass seq=SOMClass_somGetParents(somSelf);
		unsigned int i;
		SOMClass SOMSTAR cls=somNewClassReference(SOMMSingleInstance);

		i=seq._length;

		while (i--)
		{
			if (SOMClass_somDescendedFrom(SOMClass_somGetClass(seq._buffer[i]),cls))
			{
/*				i=seq._length;

				while (i--)
				{
					somReleaseClassReference(seq._buffer[i]);
				}*/

				SOMFree(seq._buffer);

				somReleaseClassReference(cls);

			    SOMMSingleInstance_parent_SOMClass_somClassReady(somSelf);

				return;
			}
		}

/*		i=seq._length;

		while (i--)
		{
			somReleaseClassReference(seq._buffer[i]);
		}
*/
		SOMFree(seq._buffer);
		
		somReleaseClassReference(cls);

		newPtr=(somMethodPtr)my_uninit;

		somThis->old_somUninitPtr=
			SOMClass_somFindSMethod(somSelf,&ptr_somUninit);

/*			SOMClass_somOverrideSMethod(somSelf,&ptr_somUninit,newPtr);*/

		SOMClass_somAddDynamicMethod(somSelf,&ptr_somUninit,
				&desc_somUninit,
				newPtr,0);
	}

#endif
    SOMMSingleInstance_parent_SOMClass_somClassReady(somSelf);
}

#ifdef SOMMSingleInstance_somDeallocate
#ifndef USE_APPLE_SOM  /* Apple's SOM does not allow overriding this method */
SOM_Scope void SOMLINK snglicls_somDeallocate(
	SOMMSingleInstance SOMSTAR somSelf,
	/* in */ somToken memptr)
{
    SOMMSingleInstanceData *somThis = SOMMSingleInstanceGetData(somSelf);
	SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,0);

	if (somPrivate)
	{
		if (somPrivate->singleObject==memptr)
		{
			somPrivate->singleObject=NULL;

			SOMMSingleInstancePrivateFree(somThis,somPrivate);
		}
	}

	SOMMSingleInstance_parent_SOMClass_somDeallocate(somSelf,memptr);
}
#endif
#endif

SOM_Scope void SOMLINK snglicls_sommSingleInstanceFreed(SOMClass SOMSTAR somSelf,Environment *ev,SOMObject SOMSTAR obj)
{
	SOMClass SOMSTAR single=somNewClassReference(SOMMSingleInstance);

	if (SOMObject_somIsA(somSelf,single))
	{
		SOMMSingleInstanceData *somThis=SOMMSingleInstanceGetData(somSelf);
		SOMMSingleInstancePrivateData *somPrivate=SOMMSingleInstancePrivateGetData(somThis,0);

		if (somPrivate)
		{
			if (somPrivate->singleObject==obj)
			{
				somPrivate->singleObject=NULL;

				SOMMSingleInstancePrivateFree(somThis,somPrivate);
			}
		}
	}

	somReleaseClassReference(single);
}

