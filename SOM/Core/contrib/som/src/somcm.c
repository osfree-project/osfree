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

#include <somirdll.h>

SOM_Scope SOMClass SOMSTAR SOMLINK somcm_somLoadClassFile(SOMClassMgr SOMSTAR somSelf, 
                                                                   somId classId, 
                                                                   long majorVersion, 
                                                                   long minorVersion, 
                                                                   string file)
{
	somToken mod=NULL;
	char *cls_name=somStringFromId(classId);
	char *func_name=SOMClassMgr_somGetInitFunction(somSelf);
	somTD_SOMLoadModule *load=SOMLoadModule;

	if (load && file && file[0])
	{
		if (!load(cls_name,file,func_name,majorVersion,minorVersion,&mod))
		{
			return SOMClassMgr_somClassFromId(somSelf,classId);
		}
	}

    return NULL;
}

struct somcm_somLocateClassFile
{
	Repository SOMSTAR rep;
	char *result;
	char *modifier;
	Environment *ev;
};

RHBOPT_cleanup_begin(somcm_somLocateClassFile_cleanup,pv)

struct somcm_somLocateClassFile *data=pv;

	if (data->rep)
	{
#ifdef SOMObject_release
		Environment ev;
		SOM_InitEnvironment(&ev);
		SOMObject_release(data->rep,&ev);
		SOM_UninitEnvironment(&ev);
#else
		static char *somRelease="somRelease";
		somToken token=NULL;

		somva_SOMObject_somDispatch(data->rep,&token,&somRelease,data->rep);
#endif
	}

	if (data->result)
	{
		SOMFree(data->result);
	}

	if (data->modifier)
	{
		SOMFree(data->modifier);
	}

	if (data->ev)
	{
		somExceptionFree(data->ev);
	}

RHBOPT_cleanup_end

SOM_Scope string  SOMLINK somcm_somLocateClassFile(
		SOMClassMgr SOMSTAR somSelf, 
        somId classId, 
        long majorVersion, 
        long minorVersion)
{
	string c=somStringFromId(classId);
	som_thread_globals_t *ev=SOMKERN_get_thread_globals(1);
	struct somcm_somLocateClassFile data={NULL,NULL,NULL,NULL};

	SOM_IgnoreWarning(majorVersion);
	SOM_IgnoreWarning(minorVersion);

	ev->somLocateClassFile[0]=0;

	RHBOPT_cleanup_push(somcm_somLocateClassFile_cleanup,&data);

	if (!strcmp(c,"Repository"))
	{
		strncpy(ev->somLocateClassFile,sz_dllname_somir,sizeof(ev->somLocateClassFile));
	}
	else
	{
		data.rep=SOMClassMgr__get_somInterfaceRepository(somSelf);

		if (data.rep)
		{
			Environment ev2;
			size_t len=strlen(c)+1;
static		char *method="lookup_modifier";

			SOM_InitEnvironment(&ev2);

			data.ev=&ev2;
			data.modifier=SOMMalloc(len+2);
			data.modifier[0]=':';
			data.modifier[1]=':';
			memcpy(data.modifier+2,c,len);

			somva_SOMObject_somDispatch(data.rep,(somToken *)(void *)&data.result,&method,data.rep,&ev2,data.modifier,"dllname");

			if (data.result && (NO_EXCEPTION==ev2._major))
			{
				strncpy(ev->somLocateClassFile,data.result,sizeof(ev->somLocateClassFile));
			}
		}
		else
		{
#ifdef HAVE_SNPRINTF
			snprintf
#else
			sprintf
#endif
				(ev->somLocateClassFile,
#ifdef HAVE_SNPRINTF
				sizeof(ev->somLocateClassFile),
#endif
				"%s.dll",c);
		}
	}

	RHBOPT_cleanup_pop();

	if (ev->somLocateClassFile[0]) return ev->somLocateClassFile;

	return NULL;
}

SOM_Scope void  SOMLINK somcm_somRegisterClass(
			SOMClassMgr SOMSTAR somSelf, 
			SOMClass SOMSTAR classObj)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);

	somStartCriticalSection();

	if (RHBCDR_kds_is_empty(&somThis->classList))
	{
		SOMClassList_create(somThis);
	}
	SOMClassList_add(somThis,0,SOMClass_somGetClassMtab(classObj));

	somEndCriticalSection();
}

SOM_Scope void  SOMLINK somcm_somRegisterClassLibrary(
		SOMClassMgr SOMSTAR somSelf, 
		string libraryName, 
		somMethodPtr libraryInitRtn)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(libraryInitRtn);

	somPrintf("SOMClass::somRegisterClassLibrary(%s) not implemented\n",
			libraryName);
}

SOM_Scope void  SOMLINK somcm_somUnregisterClassLibrary(
		SOMClassMgr SOMSTAR somSelf, 
		string libraryName)
{
	SOM_IgnoreWarning(somSelf);

	somPrintf("SOMClass::somUnregisterClassLibrary(%s) not implemented\n",
			libraryName);
}

SOM_Scope long  SOMLINK somcm_somUnloadClassFile(
	SOMClassMgr SOMSTAR somSelf, 
    SOMClass SOMSTAR classObj)
{
	somPrintf("SOMClass::somUnloadClassFile not implemented\n");

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(classObj);

    return 0;
}

SOM_Scope long SOMLINK somcm_somUnregisterClass(
	SOMClassMgr SOMSTAR somSelf, 
    SOMClass SOMSTAR classObj)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	somMethodTabPtr mtabs=SOMClass_somGetClassMtab(classObj);
	long count=-1;

	if (mtabs)
	{
		somStartCriticalSection();

		count=SOMClassList_remove(somThis,NULL,mtabs);

		if (!count) 
		{
			SOMClass_unregistered(classObj,mtabs);
		}

		somEndCriticalSection();
	}

	if (!count)
	{
		SOMClass_somFree(classObj);
	}

    return 0;
}

SOM_Scope void  SOMLINK somcm_somBeginPersistentClasses(
			SOMClassMgr SOMSTAR somSelf)
{
	somPrintf("SOMClass::somBeginPersistentClasses not implemented\n");

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(somSelf);
}

SOM_Scope void  SOMLINK somcm_somEndPersistentClasses(
			SOMClassMgr SOMSTAR somSelf)
{
	somPrintf("SOMClass::somEndPersistentClasses not implemented\n");

	SOM_IgnoreWarning(somSelf);
}

SOM_Scope boolean  SOMLINK somcm_somJoinAffinityGroup(
			SOMClassMgr SOMSTAR somSelf, 
			SOMClass SOMSTAR newClass, 
			SOMClass SOMSTAR affClass)
{
	somPrintf("SOMClass::somJoinAffinityGroup not implemented\n");

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(newClass);
	SOM_IgnoreWarning(affClass);

    return 0;
}

SOM_Scope string  SOMLINK somcm_somGetInitFunction(
		SOMClassMgr SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);

    return SOMClassInitFuncName();
}

SOM_Scope Repository SOMSTAR SOMLINK somcm__get_somInterfaceRepository(
		SOMClassMgr SOMSTAR somSelf)
{
	Repository SOMSTAR rep=NULL;
#ifdef RHBOPT_SHARED_DATA
	struct som_thread_globals_t *ev=SOMKERN_get_thread_globals(1);
	#define SOMKERN_repository   ev->repository
#else
	#define SOMKERN_repository   som_globals.repository
#endif

	somStartCriticalSection();

	if (!SOMKERN_repository)
	{
		SOMClass SOMSTAR cls=NULL;
		char *p="Repository";

/*		if (som_globals.closing) 
		{
			somEndCriticalSection();

			somPrintf("won't load any repository while closing\n");

			return 0;
		}
*/
		somEndCriticalSection();

		cls=SOMClassMgr_somFindClass(somSelf,&p,0,0);

		if (cls)
		{
			rep=SOMClass_somNew(cls);

#ifdef SOMClass_somRelease
			SOMClass_somRelease(cls);
#endif
		}

		somStartCriticalSection();

		if (rep)
		{
			if (SOMKERN_repository)
			{
#ifdef SOMObject_release
				Environment ev;
				somEndCriticalSection();
				SOM_InitEnvironment(&ev);
				SOMObject_release(rep,&ev);
				SOM_UninitEnvironment(&ev);
				somStartCriticalSection();
#else
				somEndCriticalSection();
				SOMObject_somFree(rep);
				somStartCriticalSection();
#endif
			}
			else
			{
				SOMKERN_repository=rep;
			}
		}
	}

	/* should be an AddRef here */

	rep=SOMKERN_repository;

	if (rep)
	{
#ifdef SOMObject_duplicate
		Environment ev;
		SOM_InitEnvironment(&ev);
		rep=SOMObject_duplicate(rep,&ev);
		SOM_UninitEnvironment(&ev);
#else
	#ifdef SOMObject_somDuplicateReference
		rep=SOMObject_somDuplicateReference(rep);
	#else
		#ifdef SOMRefObject_somDuplicateReference
			somTD_SOMRefObject_somDuplicateReference td=
					(somTD_SOMRefObject_somDuplicateReference)
		#else
			somTD_SOMClass_somNew td=
					(somTD_SOMClass_somNew)
		#endif
						somResolveByName(rep,"somDuplicateReference");
		if (td) rep=td(rep);
	#endif				
#endif
	}

	somEndCriticalSection();

	return rep;
}

SOM_Scope void  SOMLINK somcm__set_somInterfaceRepository(
		   SOMClassMgr SOMSTAR somSelf, 
           Repository SOMSTAR somInterfaceRepository)
{
#ifdef RHBOPT_SHARED_DATA
	struct som_thread_globals_t *ev=SOMKERN_get_thread_globals((boolean)(somInterfaceRepository ? 1 : 0));
	if (!ev) return;
#endif

	somStartCriticalSection();

	if (SOMKERN_repository != somInterfaceRepository)
	{
		SOMObject SOMSTAR old=SOMKERN_repository;

		SOMKERN_repository=NULL;

		if (somInterfaceRepository)
		{
#ifdef SOMObject_duplicate
			Environment ev2;
			SOM_InitEnvironment(&ev2);
			SOMKERN_repository=SOMObject_duplicate(somInterfaceRepository,&ev2);
			SOM_UninitEnvironment(&ev2);
#else
	#ifdef SOMObject_somDuplicateReference
			SOMKERN_repository=SOMObject_somDuplicateReference(somInterfaceRepository);
	#else
		#ifdef SOMRefObject_somDuplicateReference
			somTD_SOMRefObject_somDuplicateReference td=(somTD_SOMRefObject_somDuplicateReference)
		#else
			somTD_SOMClass_somNew td=(somTD_SOMClass_somNew)
		#endif
				somResolveByName(somInterfaceRepository,"somDuplicateReference");

			if (td)
			{
				SOMKERN_repository=td(somInterfaceRepository);
			}
			else
			{
				SOMKERN_repository=somInterfaceRepository;
			}
	#endif
#endif
		}

		if (old)
		{
#ifdef SOMObject_release
			Environment ev;
			SOM_InitEnvironment(&ev);
			SOMObject_release(old,&ev);
			SOM_UninitEnvironment(&ev);
#else
			SOMObject_somFree(old);
#endif
		}
	}

	somEndCriticalSection();

	SOM_IgnoreWarning(somSelf);
}

SOM_Scope _IDL_SEQUENCE_SOMClass  SOMLINK somcm__get_somRegisteredClasses(
		SOMClassMgr SOMSTAR somSelf)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	_IDL_SEQUENCE_SOMClass ret={0,0,NULL};

	somStartCriticalSection();

	if (!RHBCDR_kds_is_empty(&somThis->classList))
	{
		unsigned int i=RHBCDR_kds_locked_length(&somThis->classList);
		ret._buffer=SOMMalloc(sizeof(SOMClass SOMSTAR)*i);
		if (ret._buffer)
		{
			ret._length=i;
			ret._maximum=i;
			while (i--)
			{
				RHBSOMUT_KeyData *kd=RHBCDR_kds_locked_get(&somThis->classList,i);
				somMethodTabPtr mtab=kd->data._value;
				SOMClass SOMSTAR cls=mtab->classObject;
#ifdef SOMClass_somDuplicateReference
				ret._buffer[i]=SOMClass_somDuplicateReference(cls);
#else
				ret._buffer[i]=cls;
#endif
			}
		}
	}
	else
	{
		/* the prior to initialisation case... */
		unsigned int i=SOM_Bootstrap_ccds_seq._length;
		ret._buffer=SOMCalloc(i,sizeof(ret._buffer[0]));
		if (ret._buffer)
		{
			ret._length=i;
			ret._maximum=i;
			while (i--)
			{
				SOMClass SOMSTAR cls=SOM_Bootstrap_ccds_seq._buffer[i]->parentMtab->classObject;
#ifdef SOMClass_somDuplicateReference
				ret._buffer[i]=SOMClass_somDuplicateReference(cls);
#else
				ret._buffer[i]=cls;
#endif
			}
		}
	}

	somEndCriticalSection();

	return ret;
}

SOM_Scope SOMClassMgr_SOMClassArray SOMLINK somcm_somGetRelatedClasses(
		SOMClassMgr SOMSTAR somSelf, 
		SOMClass SOMSTAR classObj)
{
	somPrintf("SOMClassMgr::somGetRelatedClasses not implemented\n");

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(classObj);

    return 0;
}

SOM_Scope SOMClass SOMSTAR  SOMLINK somcm_somClassFromId(
		SOMClassMgr SOMSTAR somSelf, 
        somId classId)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	somMethodTabPtr mtab=SOMClassList_find(somThis,NULL,somStringFromId(classId));

	if (mtab)
	{
		mtab=SOMKERN_resolve_mtab(mtab);

#ifdef SOMClass_somDuplicateReference
		return SOMClass_somDuplicateReference(mtab->classObject);
#else
		return mtab->classObject;
#endif
	}

    return NULL;
}

SOM_Scope SOMClass SOMSTAR SOMLINK somcm_somFindClass(
		SOMClassMgr SOMSTAR somSelf, 
		somId classId, 
		long majorVersion, 
		long minorVersion)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	char *file=NULL;
	SOMClass SOMSTAR cls=NULL;
	somMethodTabPtr mtab=NULL;

	if (!classId) return 0;

	mtab=SOMClassList_find(somThis,0,somStringFromId(classId));

	if (mtab)
	{
		mtab=SOMKERN_resolve_mtab(mtab);

#ifdef SOMClass_somDuplicateReference
		return SOMClass_somDuplicateReference(mtab->classObject);
#else
		return mtab->classObject;
#endif
	}

	file=SOMClassMgr_somLocateClassFile(somSelf,classId,majorVersion,minorVersion);

	cls=SOMClassMgr_somFindClsInFile(somSelf,classId,majorVersion,minorVersion,file);

    return cls;
}

SOM_Scope SOMClass SOMSTAR  SOMLINK somcm_somFindClsInFile(
		SOMClassMgr SOMSTAR somSelf, 
		somId classId, 
		long majorVersion, 
		long minorVersion, 
		string file)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	somMethodTabPtr mtab=NULL;

	if (!classId) return NULL;

	mtab=SOMClassList_find(somThis,NULL,somStringFromId(classId));

	if (mtab)
	{
		mtab=SOMKERN_resolve_mtab(mtab);

#ifdef SOMClass_somDuplicateReference
		return SOMClass_somDuplicateReference(mtab->classObject);
#else
		return mtab->classObject;
#endif
	}

/*	if (som_globals.closing)
	{
		somPrintf("somFindClsInFile called during closing\n");
		return 0;
	}
*/
	return SOMClassMgr_somLoadClassFile(somSelf,classId,majorVersion,minorVersion,file);
}

/**************************************************
 *
 * hand over control to a new class mgr object
 *
 *
 */

SOM_Scope void  SOMLINK somcm_somMergeInto(
		SOMClassMgr SOMSTAR somSelf, 
		SOMObject SOMSTAR targetObj)
{
	RHBOPT_ASSERT(somSelf==SOMClassMgrObject)
	RHBOPT_ASSERT(targetObj)
	RHBOPT_ASSERT(SOMClassMgr_somIsA(targetObj,_SOMClassMgr));

	if ((somSelf==SOMClassMgrObject)&&targetObj)
	{
		SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
		SOMClassMgrData *somThat=SOMClassMgrGetData(targetObj);
		_IDL_SEQUENCE_RHBSOMUT_KeyDataPtr seq={0,0,NULL};

		somStartCriticalSection();

		somThat->classList.dataset=somThis->classList.dataset;
		somThis->classList.dataset=seq;

		SOMClassMgrObject=targetObj;

		somEndCriticalSection();

		SOMClassMgr_somFree(somSelf);
	}
}

SOM_Scope long  SOMLINK somcm_somSubstituteClass(
		SOMClassMgr SOMSTAR somSelf, 
		string origClassName, 
		string newClassName)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	somMethodTabPtr original_class=NULL;
	somMethodTabPtr replace_class=NULL;

	if (somCompareIds(&origClassName,&newClassName))
	{
#ifdef _DEBUG
		somPrintf("SOMClassMgr::somSubstituteClass(%s,%s) are the same\n",
				origClassName,newClassName);
#endif

		return -1;
	}

	original_class=SOMClassList_find(somThis,0,origClassName);
	replace_class=SOMClassList_find(somThis,0,newClassName);

	if ((!original_class)||(!replace_class))
	{
#ifdef _DEBUG
		somPrintf("SOMClassMgr::somSubstituteClass failed \n");
		if (!original_class) somPrintf("%s was not found\n",origClassName);
		if (!replace_class) somPrintf("%s was not found\n",newClassName);
#endif
		return -1;
	}

	RHBOPT_ASSERT(!somcm_is_kernel_class(original_class));
	RHBOPT_ASSERT(!somcm_is_kernel_class(replace_class));

	if ((!somcm_is_kernel_class(original_class))&&
		(!somcm_is_kernel_class(original_class)))
	{
		unsigned int i=0;

		while (i < somClassInfoFromMtab(replace_class)->classes._length)
		{
			if (somClassInfoFromMtab(replace_class)->classes._buffer[i].cls==original_class)
			{
				somClassInfoFromMtab(original_class)->substituted_mtab=replace_class;

				if (somClassInfoFromMtab(original_class)->sci)
				{
					if (somClassInfoFromMtab(original_class)->sci->cds)
					{
						somClassInfoFromMtab(original_class)->sci->cds->classObject=
								replace_class->classObject;
					}

					/* should also replace the mtab field in the 
						ccds as well, however must not break
						the class or stop somParentsNumResolve (and others?) working

					*/
				}

				return 0;
			}

			i++;
		}

#ifdef _DEBUG
		somPrintf("SOMClassMgr::somSubstituteClass, %s is not derived from %s\n",
				replace_class->className,
				original_class->className);
#endif
	}

    return -1;
}


SOM_Scope void  SOMLINK somcm_somDumpSelfInt(SOMClassMgr SOMSTAR somSelf, 
                                                            long level)
{
	SOMClassMgrData *somThis=SOMClassMgrGetData(somSelf);
	unsigned int i=0;

	somStartCriticalSection();

	if (RHBCDR_kds_is_empty(&somThis->classList))
	{
		SOMClassList_create(somThis);
	}

	while (i < RHBCDR_kds_locked_length(&somThis->classList))
	{
		RHBSOMUT_KeyData *kd=RHBCDR_kds_locked_get(&somThis->classList,i++);
		somMethodTabPtr p=kd->data._value;

		somPrefixLevel(level);
		somPrintf("%s(%ld,%ld)",
				p->className,
				(long)somClassInfoFromMtab(p)->major_version,
				(long)somClassInfoFromMtab(p)->minor_version);
		somPrintf("\n");

		if (somClassInfoFromMtab(p)->substituted_mtab != p)
		{
			somPrefixLevel(level+1);
			p=somClassInfoFromMtab(p)->substituted_mtab;
			somPrintf(" substituted with %s(%ld,%ld)",
				p->className,
				(long)somClassInfoFromMtab(p)->major_version,
				(long)somClassInfoFromMtab(p)->minor_version);
			somPrintf("\n");
		}

		somPrefixLevel(level+1);
		somPrintf(" metaclass=%s",SOMClass_somGetClassName(p->classObject));
		somPrintf("\n");
	}

	somEndCriticalSection();
}

#ifdef SOMClassMgr_somImportObject
SOM_Scope boolean SOMLINK somcm_somImportObject(
	SOMClassMgr SOMSTAR somSelf,
	/* in */ SOMObject SOMSTAR objToBeShared)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(objToBeShared);

	return 0;
}
#endif

static void SOMClassList_create(SOMClassMgrData *somThis)
{
	if (RHBCDR_kds_is_empty(&somThis->classList))
	{
		unsigned int i=0;

		RHBOPT_ASSERT(somThis->classList.lpVtbl)

/*		RHBCDR_kds_init(&somThis->data.somRegisteredClasses);*/

		while (i < SOM_Bootstrap_ccds_seq._length)
		{
			/* 20070830 */

			somClassInfo ci=somClassInfoFromParentMtabs(SOM_Bootstrap_ccds_seq._buffer[i]->parentMtab);
			somMethodTabPtr ptr=somMtabFromClassInfo(ci);

			RHBOPT_ASSERT(somMtabFromParentMtabs(SOM_Bootstrap_ccds_seq._buffer[i]->parentMtab)==ptr);

			RHBOPT_ASSERT(SOM_Bootstrap_ccds_seq._buffer[i]->parentMtab->mtab==ptr);

			SOMClassList_add(somThis,0,ptr);
			i++;
		}
	}
}

static somMethodTabPtr SOMClassList_find(SOMClassMgrData *somThis,Environment *ev,char *p)
{
	somMethodTabPtr result=NULL;

	SOM_IgnoreWarning(ev);

	if (p)
	{
		RHBSOMUT_Key key;
		RHBSOMUT_KeyData *data;

		somStartCriticalSection();

		if (RHBCDR_kds_is_empty(&somThis->classList))
		{
			SOMClassList_create(somThis);
		}

		key._buffer=(octet *)p;
		key._length=(long)strlen(p);
		key._maximum=key._length;

		data=RHBCDR_kds_find(&somThis->classList,&key);

		if (data)
		{
			result=data->data._value;
		}

		somEndCriticalSection();
	}

	return result;
}

static long SOMClassList_remove(SOMClassMgrData *somThis,Environment *ev,somMethodTabPtr mtab)
{
	long retVal=-1;

	SOM_IgnoreWarning(ev);

	if (RHBCDR_kds_count(&somThis->classList))
	{
		RHBSOMUT_KeyData *kdp=&somClassInfoFromMtab(mtab)->keyed_data;

		RHBCDR_kds_remove(&somThis->classList,kdp);

		retVal=kdp->count;
	}

	return retVal;
}

static void SOMClassList_add(SOMClassMgrData *somThis,Environment *ev,somMethodTabPtr mtab)
{
	SOM_IgnoreWarning(ev);

	RHBCDR_kds_add(&somThis->classList,&somClassInfoFromMtab(mtab)->keyed_data);
}

static void SOMClassList_destroy(SOMClassMgrData *somThis)
{
	if (somThis->classList.lpVtbl)
	{
		RHBCDR_kds_uninit(&somThis->classList);
	}
}

SOM_Scope void SOMLINK somcm_somUninit(SOMClassMgr SOMSTAR somSelf)
{
	/**********************************
	 *
	 * this should never get called as we are using somDestruct
	 *
	 */

	RHBOPT_ASSERT(!somSelf);
}

