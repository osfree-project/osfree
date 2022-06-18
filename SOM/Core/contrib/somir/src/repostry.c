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

#ifdef _PLATFORM_MACINTOSH_
	#include <MacTypes.h>
	#include <Files.h>
#endif

#include <rhbopt.h>
#define Repository_Class_Source
#define M_Repository_Class_Source

#ifdef _WIN32
	#include <windows.h>
#endif

#include <rhbsomir.h>
#include <repostry.ih>

#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
		#ifdef RHBMUTEX_INIT_DATA
			struct rhbmutex_t somir_crit=RHBMUTEX_INIT_DATA;
		#else
			struct rhbmutex_t somir_crit;
		#endif
	#else
		CRITICAL_SECTION somir_crit;
	#endif
#endif

static char *dupl_string(char *p)
{
	if (p)
	{
		long ul=(long)(strlen(p)+1);
		char *q=SOMMalloc(ul);
		memcpy(q,p,ul);
		return q;
	}

	return NULL;
}


static SOMClass SOMSTAR getRepositoryClassObject(void)
{
	return somNewClassReference(Repository);
}

static SOMClass SOMSTAR getInterfaceDefClassObject(void)
{
	return somNewClassReference(InterfaceDef);
}

static SOMClass SOMSTAR getOperationDefClassObject(void)
{
	return somNewClassReference(OperationDef);
}

static SOMClass SOMSTAR getModuleDefClassObject(void)
{
	return somNewClassReference(ModuleDef);
}

static SOMClass SOMSTAR getAttributeDefClassObject(void)
{
	return somNewClassReference(AttributeDef);
}

static SOMClass SOMSTAR getConstantDefClassObject(void)
{
	return somNewClassReference(ConstantDef);
}

static SOMClass SOMSTAR getExceptionDefClassObject(void)
{
	return somNewClassReference(ExceptionDef);
}

static SOMClass SOMSTAR getTypeDefClassObject(void)
{
	return somNewClassReference(TypeDef);
}

static SOMClass SOMSTAR getParameterDefClassObject(void)
{
	return somNewClassReference(ParameterDef);
}

static SOMClass SOMSTAR getNULL(void)
{
	return NULL;
}

struct IRtype_map
{
	const char *name;
	short type;
	SOMClass SOMSTAR (*getClassObject)(void);
};

static struct IRtype_map type_map[]={
	{"OperationDef",kSOMIR_OperationDef,getOperationDefClassObject},
	{"ParameterDef",kSOMIR_ParameterDef,getParameterDefClassObject},
	{"InterfaceDef",kSOMIR_InterfaceDef,getInterfaceDefClassObject},
	{"ExceptionDef",kSOMIR_ExceptionDef,getExceptionDefClassObject},
	{"AttributeDef",kSOMIR_AttributeDef,getAttributeDefClassObject},
	{"all",kSOMIR_Empty,getNULL},
	{"Repository",kSOMIR_Container,getRepositoryClassObject},
	{"ModuleDef",kSOMIR_ModuleDef,getModuleDefClassObject},
	{"ConstantDef",kSOMIR_ConstantDef,getConstantDefClassObject},
	{"TypeDef",kSOMIR_TypeDef,getTypeDefClassObject}
};

SOM_Scope void SOMLINK repostry_somDumpSelf(
	Repository SOMSTAR somSelf,
	/* in */ long level)
{
	Repository_parent_Container_somDumpSelf(somSelf,level);
}

SOM_Scope void SOMLINK repostry_somDumpSelfInt(
	Repository SOMSTAR somSelf,
	/* in */ long level)
{
	Repository_parent_Container_somDumpSelfInt(somSelf,level);
}

struct lookup_id
{
	RepositoryData *somThis;
	struct SOMIR_ContainedData *retVal;
	int is_using;
};

RHBOPT_cleanup_begin(lookup_id_cleanup,pv)

struct lookup_id *data=pv;

	if (!data->is_using)
	{
		data->somThis->data_set.lpVtbl->end_use(&(data->somThis->data_set));
	}

RHBOPT_cleanup_end

static struct SOMIR_ContainedData *lookup_id(
		Repository somSelf,
		RepositoryData *somThis,
		Environment *ev,
		char *search_id)
{
	struct lookup_id data={NULL,NULL,0};

	data.somThis=somThis;

	RHBOPT_cleanup_push(lookup_id_cleanup,&data);
		
	data.is_using=somThis->data_set.lpVtbl->begin_use(&somThis->data_set);

	if (!data.is_using)
	{
		struct SOMIRfile *f=somThis->data_set.first;

		while (f)
		{
			data.retVal=f->lpVtbl->Lookup(f,search_id);

			if (data.retVal) break;

			f=f->next;
		}

	}

	RHBOPT_cleanup_pop();

	if (!data.retVal)
	{
#if 0
		somPrintf("lookup_id(%s) failed\n",search_id);
#endif

		RHBOPT_throw_StExcep(ev,INTF_REPOS,IRNotFound,NO);
	}

	return data.retVal;
}

SOM_Scope corbastring SOMLINK repostry_lookup_modifier(
	Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring name,
	/* in */ corbastring modifier)
{
	RepositoryData *somThis=RepositoryGetData(somSelf);
	struct SOMIR_ContainedData *cnd=lookup_id(somSelf,somThis,ev,name);
	char *retVal=NULL;

	if (cnd && (ev->_major==NO_EXCEPTION))
	{
		unsigned long i=cnd->modifiers._length;
		somModifier *m=cnd->modifiers._buffer;

		while (i--)
		{
			if (!strcmp(m->name,modifier))
			{
				retVal=dupl_string(m->value ? m->value : "");
				break;
			}

			m++;
		}

		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope Contained SOMSTAR SOMLINK repostry_lookup_id(
	Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring search_id)
{
	RepositoryData *somThis=RepositoryGetData(somSelf);
	struct SOMIR_ContainedData *cnd=lookup_id(somSelf,somThis,ev,search_id);
	Contained SOMSTAR retVal=NULL;

	if (cnd)
	{
		retVal=somThis->data_set.lpVtbl->Wrap(cnd,ev);

		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}


SOM_Scope void SOMLINK repostry_release_cache(
	Repository SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
}

SOM_Scope boolean SOMLINK repostry_queryException(
	Repository SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return 0;
}

SOM_Scope _IDL_SEQUENCE_Container_ContainerDescription SOMLINK repostry_describe_contents(
	Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ Container_InterfaceName limit_type,
	/* in */ boolean exclude_inherited,
	/* in */ long max_returned_objs)
{
	return Repository_parent_Container_describe_contents(
		somSelf,ev,limit_type,exclude_inherited,max_returned_objs);
}

static Contained SOMSTAR repostry_Wrap(struct SOMIR_ContainedData *somThis,Environment *ev)
{
	Contained SOMSTAR retVal=NULL;

	if (somThis)
	{
		struct IRtype_map *p=type_map;
		unsigned int i=sizeof(type_map)/sizeof(type_map[0]);

		while (i--)
		{
			if (p->type==somThis->type)
			{
				SOMClass SOMSTAR cls=p->getClassObject();

				if (cls)
				{
					retVal=SOMClass_somNew(cls);

					somReleaseClassReference(cls);

					if (retVal)
					{
						Contained__set_containedData(retVal,ev,somThis);
					}

					break;
				}
			}

			p++;
		}
	}

	return retVal;
}

static short repostry_TypeFromName(const char *name)
{
	short retVal=kSOMIR_Empty;

	if (name)
	{
		struct IRtype_map *p=type_map;
		unsigned int i=sizeof(type_map)/sizeof(type_map[0]);

		while (i--)
		{
			if (!strcmp(name,p->name))
			{
				retVal=p->type;
				break;
			}

			p++;
		}
	}

	return retVal;
}

SOM_Scope SOMObject SOMSTAR SOMLINK m_repostry_somNew(M_Repository SOMSTAR somSelf)
{
	M_RepositoryData *somThis=M_RepositoryGetData(somSelf);
	Repository SOMSTAR result=NULL;

	SOMIR_LOCK

	if (somThis->singleObject.repositoryObject)
	{
		result=Repository_somDuplicateReference(somThis->singleObject.repositoryObject);
	}
	else
	{
#ifdef M_Repository_parent_SOMMSingleInstance_somNew
		result=M_Repository_parent_SOMMSingleInstance_somNew(somSelf);
#else
		somThis->singleObject.repositoryObject=M_Repository_parent_SOMClass_somNew(somSelf);
		result=somThis->singleObject.repositoryObject;
#endif
	}

	SOMIR_UNLOCK

	return result;
}

SOM_Scope void SOMLINK m_repostry_somUninit(M_Repository SOMSTAR somSelf)
{
	M_RepositoryData *somThis=M_RepositoryGetData(somSelf);
	Repository SOMSTAR obj=NULL;

	SOMIR_LOCK

	obj=somThis->singleObject.repositoryObject;
	somThis->singleObject.repositoryObject=NULL;

	SOMIR_UNLOCK

	if (obj) 
	{
		if (somIsObj(obj))
		{
#ifdef _DEBUG
			somPrintf("Repository object %p still left over\n",obj);
			SOMObject_somPrintSelf(obj);
#endif

			Repository_somRelease(obj);
		}
		else
		{
			somPrintf("Repository object pointer %p still left over\n",obj);
		}
	}

	M_Repository_parent_SOMClass_somUninit(somSelf);
}

static int repostry_begin_use(struct SOMIR_Repository *somThis)
{
	RHBOPT_ASSERT(somThis);

	rhbatomic_inc(&(somThis->inUse));

	SOMIR_LOCK

	return 0;
}

static void repostry_end_use(struct SOMIR_Repository *somThis)
{
	RHBOPT_ASSERT(somThis);

	if (!rhbatomic_dec(&(somThis->inUse)))
	{
		/* close all files */

		struct SOMIRfile *f=somThis->first;

		while (f)
		{
			f->lpVtbl->Close(f);

			f=f->next;
		}
	}

	SOMIR_UNLOCK
}

static struct SOMIR_RepositoryVtbl repositoryVtbl={
	repostry_Wrap,
	repostry_TypeFromName,
	repostry_begin_use,
	repostry_end_use
};

static void repositry_common_init(Repository SOMSTAR somSelf,RepositoryData *somThis)
{
	char *irEnv=irGetFileNames();

	somThis->data_set.lpVtbl=&repositoryVtbl;

	if (irEnv)
	{
		struct
		{
			unsigned long _length,_maximum;
			char *_buffer;
		} data={0,0,NULL};
		char *p;
		char *filename;

		data._maximum=(long)(1+strlen(irEnv));
		data._buffer=SOMMalloc(data._maximum);

		memcpy(data._buffer,irEnv,data._maximum);

		p=filename=data._buffer;

		while (filename)
		{
			while (*p)
			{
				if (*p==kSOMIRseparator)
				{
					*p++=0;

					if (!*p) 
					{
						p=NULL;
					}

					break;
				}

				p++;
			}

			if (p && !*p) p=NULL;

			if (*filename)
			{
				SOMIRopen(&somThis->data_set,filename);
			}

			filename=p;
		}

		SOMFree(data._buffer);
	}
}

static void repositry_common_uninit(
	Repository SOMSTAR somSelf,
	RepositoryData *somThis)
{
	while (somThis->data_set.first)
	{
		struct SOMIRfile *f=somThis->data_set.first;

		f->lpVtbl->Detach(f);

		RHBOPT_ASSERT(f!=somThis->data_set.first);

		f->lpVtbl->Release(f);
	}
}

SOM_Scope void SOMLINK repostry_somInit(
	Repository SOMSTAR somSelf)
{
	Repository_parent_Container_somInit(somSelf);
	repositry_common_init(somSelf,RepositoryGetData(somSelf));
}

SOM_Scope void SOMLINK repostry_somUninit(
	Repository SOMSTAR somSelf)
{
	RepositoryData *somThis=RepositoryGetData(somSelf);
	SOMClass SOMSTAR classObject=somSelf->mtab->classObject;

	SOMIR_LOCK

	if (classObject)
	{
		SOMClass SOMSTAR cls=somNewClassReference(M_Repository);

		if (cls)
		{
			if (SOMObject_somIsA(classObject,cls))
			{
				M_RepositoryData *somThis=M_RepositoryGetData(classObject);

				if (somThis && (somThis->singleObject.repositoryObject==somSelf))
				{
					somThis->singleObject.repositoryObject=NULL;
				}

			}

			somReleaseClassReference(cls);
		}
	}

	repositry_common_uninit(somSelf,somThis);

	SOMIR_UNLOCK

	Repository_parent_Container_somUninit(somSelf);
}

SOM_Scope _IDL_SEQUENCE_Contained SOMLINK repostry_contents(
	Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ Container_InterfaceName limit_type,
	/* in */ boolean exclude_inherited)
{
	RepositoryData *somThis=RepositoryGetData(somSelf);
	_IDL_SEQUENCE_Contained seq={0,0,NULL};

	if (!somThis->data_set.lpVtbl->begin_use(&somThis->data_set))
	{
		struct SOMIRfile *f=somThis->data_set.first;
		short type=somThis->data_set.lpVtbl->TypeFromName(limit_type);

		while (f && (ev->_major==NO_EXCEPTION))
		{
			struct SOMIR_ContainerData *cnr=&(f->containerData);
			struct SOMIR_ContainerItem *p=cnr->itemSeq._buffer;
			unsigned long i=cnr->itemSeq._length;

			while (i-- && (ev->_major==NO_EXCEPTION))
			{
				if ((type==kSOMIR_Empty)||(type==p->type))
				{
					unsigned long j=seq._length;
					boolean doAdd=1;

					while (j--)
					{
						Contained SOMSTAR cnd=seq._buffer[j];
						char *name=Contained__get_name(cnd,ev);

						RHBOPT_ASSERT(cnd);
						RHBOPT_ASSERT(name);

						if (!strcmp(name,p->name))
						{
							doAdd=0;

							break;
						}
					}

					if (doAdd)
					{
						struct SOMIR_ContainedData *cnd=cnr->lpVtbl->Acquire(cnr,p->offset,p->type);

						RHBOPT_ASSERT(cnd);

						if (cnd)
						{
							Contained SOMSTAR obj=somThis->data_set.lpVtbl->Wrap(cnd,ev);

							RHBOPT_ASSERT(obj);

							cnd->lpVtbl->Release(cnd);

							if (obj)
							{
								boolean b=Repository_seqContained_add(somSelf,ev,&seq,obj);

								if (ev->_major != NO_EXCEPTION)
								{
									Contained_somRelease(obj);

									break;
								}

								if (!b)
								{
									Contained_somRelease(obj);
								}
							}
						}
					}
				}

				p++;
			}

			f=f->next;
		}

		somThis->data_set.lpVtbl->end_use(&somThis->data_set);
	}

	return seq;
}

SOM_Scope Contained SOMSTAR SOMLINK repostry_find(Repository SOMSTAR somSelf,Environment *ev,char * absolute_name)
{
/*	RepositoryData *somThis=RepositoryGetData(somSelf);*/
	Contained SOMSTAR obj=NULL;

	return obj;
}



SOM_Scope void SOMLINK repostry_somDestruct(
	Repository SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	RepositoryData *somThis;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	Repository_BeginDestructor

	RHBOPT_unused(somThis);

	repostry_somUninit(somSelf);

	Repository_EndDestructor
}

SOM_Scope void SOMLINK m_repostry_somDestruct(
	M_Repository SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	M_RepositoryData *somThis;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	M_Repository_BeginDestructor

	RHBOPT_unused(somThis);

	m_repostry_somUninit(somSelf);

	M_Repository_EndDestructor
}

static void SOMLINK m_repostry_clsInit(SOMClass SOMSTAR somSelf)
{
	SOMIR_DLL_INIT
}

