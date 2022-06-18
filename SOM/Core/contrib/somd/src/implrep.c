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

#define ImplRepository_VA_STUBS
#define ImplRepository_Class_Source

#include <rhbopt.h>
#include <rhbsomd.h>
#include <rhbgiops.h>
#include <somestio.h>

#include "implrep.ih"

#if  !defined(ImplRepository_VA_STUBS) || defined(USE_APPLE_SOM)
static boolean va_ImplRepository_somddDispatch(
		ImplRepository SOMSTAR somSelf,
		Environment *ev,
		somToken *retValue,
		corbastring method,
		...)
{
	boolean __result;
	va_list ap;
	va_start(ap,method);
	__result=ImplRepository_somddDispatch(somSelf,ev,retValue,method,ap);
	va_end(ap);
	return __result;
}
#endif

static void ImplRepository_localize_group(
		ImplRepository SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_ImplementationDef *group);

SOM_Scope ImplementationDef SOMSTAR  SOMLINK implrep_find_impldef(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		ImplId implid)
{
	ImplementationDef SOMSTAR __result=NULL;
	RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

	if (ev->_major!=NO_EXCEPTION) 
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadEnvironment,NO);

		return NULL;
	}

	if ((!implid)||(!implid[0]))
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,ImplNotFound,NO);

		return NULL;
	}

	if (!strcmp(implid,RHBORB_get_somdd_implid(orb)))
	{
		return ImplRepository__get_SOMDD_ImplDefObject(somSelf,ev);
	}

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,"find_impldef",
			somSelf,ev,implid);

	if (!ev->_major)
	{
		_IDL_SEQUENCE_ImplementationDef seq={1,1,NULL};
		seq._buffer=&__result;
		ImplRepository_ensure_local(somSelf,ev,&seq);
	}

	return __result;
}

SOM_Scope ImplementationDef SOMSTAR  SOMLINK implrep_find_impldef_by_alias(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		corbastring alias_name)
{
	ImplementationDef SOMSTAR __result=NULL;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"find_impldef_by_alias",
			somSelf,ev,alias_name);

	if (!ev->_major)
	{
		_IDL_SEQUENCE_ImplementationDef seq={1,1,NULL};
		seq._buffer=&__result;
		ImplRepository_ensure_local(somSelf,ev,&seq);
	}

    return __result;
}

SOM_Scope ORBStatus  SOMLINK implrep_find_all_impldefs(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		_IDL_SEQUENCE_ImplementationDef* outimpldefs)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"find_all_impldefs",
			somSelf,ev,outimpldefs);

	if (!ev->_major)
	{
		ImplRepository_localize_group(somSelf,ev,outimpldefs);
	}

    return __result;
}

static boolean is_SOMDD_class(char *p)
{
	if (!strcmp(p,"ImplRepository")) return 1;
	return 0;
}

SOM_Scope _IDL_SEQUENCE_ImplementationDef SOMLINK implrep_find_impldef_by_class(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		corbastring classname)
{
	_IDL_SEQUENCE_ImplementationDef ret={0,0,NULL};
/*	ImplRepositoryData *somThis=ImplRepositoryGetData(somSelf);*/

	if (ev->_major) return ret;

	if (is_SOMDD_class(classname))
	{
		ImplementationDef SOMSTAR def=ImplRepository__get_SOMDD_ImplDefObject(somSelf,ev);

		if (!ev->_major)
		{
			if (def)
			{
				ret._length=1;
				ret._maximum=1;
				ret._buffer=SOMMalloc(sizeof(def));
				ret._buffer[0]=def;
			}
		}

		return ret;
	}

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&ret,
			"find_impldef_by_class",
			somSelf,ev,classname);

	if (!ev->_major)
	{
		ImplRepository_localize_group(somSelf,ev,&ret);
	}

    return ret;
}


SOM_Scope ORBStatus  SOMLINK implrep_add_impldef(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		ImplementationDef SOMSTAR impldef)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"add_impldef",
			somSelf,ev,impldef);

	return __result;
}

SOM_Scope ORBStatus  SOMLINK implrep_update_impldef(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		ImplementationDef SOMSTAR impldef)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"update_impldef",
			somSelf,ev,impldef);

	return __result;
}

SOM_Scope ORBStatus  SOMLINK implrep_delete_impldef(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		ImplId implid)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"delete_impldef",
			somSelf,ev,implid);

	return __result;
}

SOM_Scope void  SOMLINK implrep_add_class_to_impldef(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		ImplId implid, 
		corbastring classname)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"add_class_to_impldef",
			somSelf,ev,implid,classname);
}

SOM_Scope void  SOMLINK implrep_remove_class_from_impldef(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		ImplId implid, 
		corbastring classname)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"remove_class_from_impldef",
			somSelf,ev,implid,classname);
}

SOM_Scope void  SOMLINK implrep_remove_class_from_all(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		corbastring classname)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"remove_class_from_all",
			somSelf,ev,classname);
}

SOM_Scope _IDL_SEQUENCE_string  SOMLINK implrep_find_classes_by_impldef(
		ImplRepository SOMSTAR somSelf, 
		Environment *ev, 
		ImplId implid)
{
	_IDL_SEQUENCE_string ret={0,0,0};

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&ret,
			"find_classes_by_impldef",
			somSelf,ev,implid);

    return ret;
}

#ifdef ImplRepository_somDefaultInit
SOM_Scope void SOMLINK implrep_somDefaultInit(
	ImplRepository SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	ImplRepositoryData *somThis; */

	ImplRepository_BeginInitializer_somDefaultInit
	ImplRepository_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}
#else
SOM_Scope void  SOMLINK implrep_somInit(
		ImplRepository SOMSTAR somSelf)
{
#ifdef ImplRepository_parent_SOMRefObject_somInit
    ImplRepository_parent_SOMRefObject_somInit(somSelf);
#else
    ImplRepository_parent_SOMObject_somInit(somSelf);
#endif
	RHBOPT_unused(somSelf)
}
#endif

#ifdef ImplRepository_somDestruct
SOM_Scope void SOMLINK implrep_somDestruct(
	ImplRepository SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	ImplRepositoryData *somThis; */

	ImplRepository_BeginDestructor
	ImplRepository_EndDestructor
}
#else
SOM_Scope void  SOMLINK implrep_somUninit(
		ImplRepository SOMSTAR somSelf)
{
	if (SOMD_ImplRepObject==somSelf)
	{
		SOMD_ImplRepObject=NULL;
	}

#ifdef ImplRepository_parent_SOMRefObject_somUninit
    ImplRepository_parent_SOMRefObject_somUninit(somSelf);
#else
    ImplRepository_parent_SOMObject_somUninit(somSelf);
#endif
}
#endif

SOM_Scope ORBStatus SOMLINK implrep_add_class_with_properties(
	ImplRepository SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplId implid,
	/* in */ corbastring classname,
	/* in */ ImplRepository_PVList *pvl)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
				(somToken *)(void *)&__result,
				"add_class_with_properties",
				somSelf,ev,implid,classname,pvl);

	return __result;
}

SOM_Scope ORBStatus SOMLINK implrep_add_class_to_all(
	ImplRepository SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring classname)
{
	ORBStatus __result=SOMDERROR_DispatchError;

	somva_ImplRepository_somddDispatch(somSelf,ev,
			(somToken *)(void *)&__result,
			"add_class_to_all",
			somSelf,ev,classname);

	return __result;
}

SOM_Scope ORBStatus SOMLINK implrep_find_all_aliases(
	ImplRepository SOMSTAR somSelf,
	Environment *ev,
	/* out */ _IDL_SEQUENCE_string *impl_aliases)
{
	ORBStatus __result=SOMDERROR_DispatchError;
	
	impl_aliases->_buffer=NULL;
	impl_aliases->_length=0;
	impl_aliases->_maximum=0;

	somva_ImplRepository_somddDispatch(somSelf,ev,
				(somToken *)(void *)&__result,
				"find_all_aliases",
				somSelf,ev,impl_aliases);

	return __result;
}

SOM_Scope ORBStatus SOMLINK implrep_ensure_remote(
	ImplRepository SOMSTAR somSelf,
	Environment *ev,
	ImplementationDef SOMSTAR *objref)
{
	return 0;
}

struct implrep_ensure_local_remote_getstring
{
	Request SOMSTAR req;
	NamedValue nv;
	char *value;
};

static ORBStatus implrep_ensure_local_remote_getstring(
		SOMDClientProxy SOMSTAR proxy,
		Environment *ev,
		struct implrep_ensure_local_remote_getstring *str,
		char *req)
{
	ORBStatus status=0;

	status=SOMDObject_create_request(proxy,ev,0,req,0,&str->nv,&str->req,0);

	if (!status)
	{
		status=Request_send(str->req,ev,0);
	}

	return status;
}


struct implrep_ensure_local_remote
{
	SOMDClientProxy SOMSTAR proxy;
	struct implrep_ensure_local_remote_getstring
			impldef_class,
			somGetClassName;
};

struct implrep_ensure_local
{
	struct
	{
		unsigned long _length,_maximum;
		struct implrep_ensure_local_remote *_buffer;
	} remotes;
	CosStream_StreamIO SOMSTAR stream;
	_IDL_SEQUENCE_ImplementationDef *locals;
	Request SOMSTAR req;
	_IDL_SEQUENCE_octet stream_data;
	somId id;
	SOMClassMgr SOMSTAR mgr;
};

RHBOPT_cleanup_begin(implrep_ensure_local_cleanup,pv)

	struct implrep_ensure_local *data=pv;

	if (data->id)
	{
		SOMFree(data->id);
		data->id=NULL;
	}

	if (data->stream_data._buffer)
	{
		SOMFree(data->stream_data._buffer);
		data->stream_data._buffer=NULL;
	}

	if (data->locals)
	{
		while (data->locals->_length)
		{
			ImplementationDef SOMSTAR impl;
			
			data->locals->_length--;
			impl=data->locals->_buffer[data->locals->_length];
			data->locals->_buffer[data->locals->_length]=NULL;

			if (impl) somReleaseObjectReference(impl);
		}
	}

	if (data->remotes._buffer)
	{
		struct implrep_ensure_local_remote *item=data->remotes._buffer;
		unsigned long i=data->remotes._length;
		while (i--)
		{
			if (item->proxy) somReleaseObjectReference(item->proxy);
			if (item->somGetClassName.req) somReleaseObjectReference(item->somGetClassName.req);
			if (item->impldef_class.req) somReleaseObjectReference(item->impldef_class.req);
			if (item->somGetClassName.value) SOMFree(item->somGetClassName.value);
			if (item->impldef_class.value) SOMFree(item->impldef_class.value);
			item++;
		}

		SOMFree(data->remotes._buffer);
		data->remotes._buffer=NULL;
	}

	if (data->req)
	{
		somReleaseObjectReference(data->req);
		data->req=NULL;
	}

	if (data->mgr)
	{
		somReleaseClassManagerReference(data->mgr);
		data->mgr=NULL;
	}

	if (data->stream)
	{
		somReleaseObjectReference(data->stream);
		data->stream=NULL;
	}

RHBOPT_cleanup_end

SOM_Scope ORBStatus SOMLINK implrep_ensure_local(
	ImplRepository SOMSTAR somSelf,
	Environment *ev,
	_IDL_SEQUENCE_ImplementationDef *seq)
{
	struct implrep_ensure_local data={{0,0,NULL},NULL,NULL,NULL,{0,0,NULL},NULL,NULL};
	unsigned long RHBOPT_volatile  i=0;
	RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);
	ORBStatus RHBOPT_volatile status=0;

	if (ev->_major) return SOMDERROR_BadEnvironment;

	if (!seq->_length) return 0;

	RHBOPT_cleanup_push(implrep_ensure_local_cleanup,&data);

	data.remotes._maximum=seq->_length;
	data.remotes._buffer=SOMMalloc(data.remotes._maximum * sizeof(data.remotes._buffer[0]));

	if (!data.remotes._buffer)
	{
		RHBOPT_throw_StExcep(ev,NO_MEMORY,UnexpectedNULL,MAYBE);
		status=SOMDERROR_BadEnvironment;
	}
	else
	{
#define init_getstring(x)    \
		(x).req=NULL; (x).value=NULL; (x).nv.argument._type=TC_string; \
		(x).nv.argument._value=&((x).value); (x).nv.len=0; (x).nv.arg_modes=0; (x).nv.name=NULL;

		while (i < data.remotes._maximum)
		{
			data.remotes._buffer[data.remotes._length].proxy=NULL;
			init_getstring(data.remotes._buffer[data.remotes._length].somGetClassName)
			init_getstring(data.remotes._buffer[data.remotes._length].impldef_class)

			data.remotes._length++;

			if (!ev->_major)
			{
				SOMObject SOMSTAR impl=seq->_buffer[i];

				if (impl)
				{
					if (RHBORB_isSOMDObject(orb,impl))
					{
						seq->_buffer[i]=NULL;
						data.remotes._buffer[i].proxy=impl;

						if (!data.stream)
						{
							data.stream=ImplRepository_create_stream(somSelf,ev);
						}

						if (!ev->_major)
						{
							implrep_ensure_local_remote_getstring(impl,ev,
									&data.remotes._buffer[i].somGetClassName,"somGetClassName");

							implrep_ensure_local_remote_getstring(impl,ev,
									&data.remotes._buffer[i].impldef_class,"_get_impldef_class");

							ImplementationDef_externalize_to_stream(impl,ev,data.stream);
						}
					}
				}
			}

			i++;
		}

		if (data.stream)
		{
			if (RHBORB_isSOMDObject(orb,data.stream))
			{
				NamedValue nv=RHBSOMD_Init_NamedValue;
				nv.argument._type=somdTC_sequence_octet;
				nv.argument._value=&data.stream_data;

				SOMDObject_create_request(data.stream,ev,0,
					"get_buffer",
					0,&nv,&data.req,0);

				if (!ev->_major)
				{
					Request_invoke(data.req,ev,0);
				}
			}
			else
			{
				data.stream_data=somStream_StreamIO_get_buffer(data.stream,ev);
			}

			if (data.stream)
			{
				Environment ev2;
				SOMObject SOMSTAR stream=data.stream;
				SOM_InitEnvironment(&ev2);
				SOMDClientProxy_somdTargetFree(data.stream,&ev2);
				somExceptionFree(&ev2);
				data.stream=NULL;
				SOMDClientProxy_somdProxyFree(stream,&ev2);
				SOM_UninitEnvironment(&ev2);
			}

		
	#ifdef somNewObject
			data.stream=somNewObject(somStream_MemoryStreamIO);
	#else
			data.stream=somStream_MemoryStreamIONew();
	#endif

			somStream_MemoryStreamIO_set_buffer(data.stream,ev,&data.stream_data);
		}

		i=0;

		while (i < data.remotes._length)
		{
			struct implrep_ensure_local_remote *item=data.remotes._buffer+i;
			SOMDClientProxy SOMSTAR remote=item->proxy;

			if (remote && !ev->_major)
			{
				ImplementationDef SOMSTAR local=NULL;
				char *impl_class=NULL;

				if (item->impldef_class.req)
				{
					Request_get_response(item->impldef_class.req,ev,0);

					if (!ev->_major)
					{
						impl_class=item->impldef_class.value;
					}
				}

				if (item->somGetClassName.req)
				{
					Request_get_response(item->somGetClassName.req,ev,0);

					if ((!impl_class) && !ev->_major)
					{
						impl_class=item->somGetClassName.value;
					}
				}

				if (impl_class)
				{
					SOMClass SOMSTAR local_class=NULL;
					data.mgr=somGetClassManagerReference();
					data.id=somIdFromString(impl_class);
					local_class=SOMClassMgr_somFindClass(data.mgr,data.id,0,0);
					somReleaseClassManagerReference(data.mgr);
					data.mgr=NULL;
					SOMFree(data.id);
					data.id=NULL;

					if (local_class)
					{
						local=SOMClass_somNew(local_class);

						somReleaseClassReference(local_class);
					}
					else
					{
						RHBOPT_throw_StExcep(ev,INV_IDENT,UnexpectedNULL,NO);
					}
				}
				
				if ((!local) && (!ev->_major))
				{
					local=SOMClass_somNew(orb->somClass_ImplementationDef_ref);
				}

				if (seq->_buffer[i])
				{
					SOMD_bomb("this should have been moved to remoted\n");
				}
				else
				{
					seq->_buffer[i]=local;
				}

				if (local && !ev->_major)
				{
					ImplementationDef_internalize_from_stream(local,ev,data.stream,0);
				}
				else
				{
					RHBOPT_throw_StExcep(ev,BAD_PARAM,ImplNotFound,NO);
				}
			}

			i++;
		}
	}

	if (ev->_major)
	{
		if (!status) status=SOMDERROR_BadEnvironment;
	}
	else
	{
		/* thank you!! */
		data.locals=NULL;
	}

	RHBOPT_cleanup_pop();

	return status;
}

SOM_Scope ImplementationDef SOMSTAR SOMLINK implrep__get_SOMDD_ImplDefObject(
	ImplRepository SOMSTAR somSelf,
	Environment *ev)
{
/*	ImplRepositoryData *somThis=ImplRepositoryGetData(somSelf);*/
	ImplementationDef SOMSTAR result=NULL;
	RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

	if (ev->_major) return NULL;

/*	somPrintf("%s:%d, making simple SOMDD impl...\n",
			__FILE__,__LINE__);
*/
	result=SOMClass_somNew(orb->somClass_ImplementationDef_ref);

	ImplementationDef__set_impl_id(result,ev,
			RHBORB_get_somdd_implid(orb));

	/* should create svr_objref from default data */

	/* get hostname */
	/* get default port */
	/* create IIOP profile data */
	/* create IOR */
	/* serialise */

	{
		IIOP_ProfileBody_1_1 body={{1,0},NULL,0,{0,0,NULL},{0,0,NULL}};
		IOP_TaggedProfile profile={IOP_TAG_INTERNET_IOP,{0,0,NULL}};
		IOP_IOR ior={NULL,{1,1,NULL}};
		any a={NULL,NULL};
		_IDL_SEQUENCE_octet ior_data={0,0,NULL};

		ior.type_id=RHBORB_get_SOMDServer_type_id(orb,ev);
		ior.profiles._buffer=&profile;

		body.host=RHBORB_get_HOSTNAME(orb);
		body.port=RHBORB_get_SOMDPORT(orb);
		body.object_key=somdGetDefaultObjectKey(ev,
				  RHBORB_get_somdd_implid(orb));

		a._value=&body;
		a._type=somdTC_IIOP_ProfileBody_1_0;

		profile.profile_data=IOP_Codec_encode_value(orb->iop_codec,ev,&a);

		a._value=&ior;
		a._type=somdTC_IOP_IOR;

		ior_data=IOP_Codec_encode_value(orb->iop_codec,ev,&a);

		if (profile.profile_data._buffer)
		{
			SOMFree(profile.profile_data._buffer);
		}

		if (body.object_key._buffer) 
		{
			SOMFree(body.object_key._buffer);
		}

		if (ior.type_id)
		{
			SOMFree(ior.type_id);
		}

		if (ior_data._length && !ev->_major)
		{
			_IDL_SEQUENCE_octet *svr_objref=SOMMalloc(sizeof(*svr_objref));
			*svr_objref=ior_data;
			ior_data._buffer=NULL;
			
			ImplementationDef__set_svr_objref(result,ev,svr_objref);
		}

	}

	return result;
}

SOM_Scope ImplRepository SOMSTAR SOMLINK implrep__get_SOMDD_ImplRepObject(
	ImplRepository SOMSTAR somSelf,
	Environment *ev)
{
/*	ImplRepositoryData *somThis=ImplRepositoryGetData(somSelf);*/
	ImplRepository SOMSTAR result=NULL;

	/* I only want the SOMDD object which has the same
		class as me, (will actually be a subclass in server process) */

	result=somdCreate(ev,ImplRepository_somGetClassName(somSelf),1);

	if (ev->_major)
	{
		result=NULL;
	}

	return result;
}

SOM_Scope SOMDServer SOMSTAR SOMLINK implrep__get_SOMDD_ServerObject(
		ImplRepository SOMSTAR somSelf,
		Environment *ev)
{
	SOMDServer SOMSTAR server=NULL;

	if (ev->_major==NO_EXCEPTION) 
	{
		if (SOMD_ObjectMgr && SOMD_ORBObject)
		{
			RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);
			char *implid=RHBORB_get_somdd_implid(orb);

			/* this should use IPC to contact somdd to get it's IOR */

			server=SOMDObjectMgr_somdFindServer(SOMD_ObjectMgr,ev,implid);
		}
		else
		{
			RHBOPT_throw_StExcep(ev,INITIALIZE,NoSOMDInit,NO);
		}
	}

	return server;
}

struct implrep_somddDispatch
{
	ImplRepository SOMSTAR rep;
	somId id;
};

RHBOPT_cleanup_begin(implrep_somddDispatch_cleanup,pv)

struct implrep_somddDispatch *data=pv;

	if (data->rep) somReleaseObjectReference(data->rep);
	if (data->id) SOMFree(data->id);

RHBOPT_cleanup_end

SOM_Scope boolean SOMLINK implrep_somddDispatch(
		ImplRepository SOMSTAR somSelf,
		Environment *ev,
		somToken *result,
		char *method,
		va_list ap)
{
	boolean RHBOPT_volatile res=0;

	if (!ev->_major)
	{
		struct implrep_somddDispatch data={NULL,NULL};

		RHBOPT_cleanup_push(implrep_somddDispatch_cleanup,&data);

		data.rep=ImplRepository__get_SOMDD_ImplRepObject(somSelf,ev);

		if (data.rep && !ev->_major)
		{
			void *oldTarget=somvalistGetTarget(ap);

			if (oldTarget==somSelf)
			{
				data.id=somIdFromString(method);
				somvalistSetTarget(ap,data.rep);
				res=SOMObject_somDispatch(data.rep,result,data.id,ap);
			}
			else
			{
				SOMD_bomb("error in va_list");
			}
		}

		RHBOPT_cleanup_pop();
	}

	return res;
}

struct implrep_create_stream
{
	SOMDServer SOMSTAR server;
	CosStream_StreamIO SOMSTAR strm;
	Request SOMSTAR req;
	TypeCode tc;
	char *cls;
};

RHBOPT_cleanup_begin(implrep_create_stream_cleanup,pv)

struct implrep_create_stream *data=pv;

	if (data->server) somReleaseObjectReference(data->server);
	if (data->strm) somReleaseObjectReference(data->strm);
	if (data->req) somReleaseObjectReference(data->req);
	if (data->tc)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		TypeCode_free(data->tc,&ev);
		SOM_UninitEnvironment(&ev);
	}
	if (data->cls) SOMFree(data->cls);

RHBOPT_cleanup_end

SOM_Scope CosStream_StreamIO SOMSTAR SOMLINK implrep_create_stream(
		ImplRepository SOMSTAR somSelf,
		Environment *ev)
{
struct implrep_create_stream data={NULL,NULL,NULL,NULL,NULL};
	RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);
	char *className="somStream::MemoryStreamIO";
	char *m="somdCreateObj";
	char *hint=NULL;
	CosStream_StreamIO SOMSTAR RHBOPT_volatile result=NULL;

	if (ev->_major) return NULL;
	
	RHBOPT_cleanup_push(implrep_create_stream_cleanup,&data);

	data.server=ImplRepository__get_SOMDD_ServerObject(somSelf,ev);

	if (data.server && (!ev->_major) && orb)
	{
		if (RHBORB_isSOMDObject(orb,data.server))
		{
			NamedValue nv=RHBSOMD_Init_NamedValue;

			data.tc=TypeCodeNew(tk_objref,"::somStream::StreamIO");

			nv.argument._type=data.tc;
			nv.argument._value=&data.strm;

			if (!SOMDObject_create_request(data.server,ev,0,
					m,
					0,&nv,&data.req,0))
			{
				Request_add_arg(data.req,ev,"class",TC_string,&className,0,ARG_IN);
				Request_add_arg(data.req,ev,"hint",TC_string,&hint,0,ARG_IN);
				Request_invoke(data.req,ev,0);
			}

			if (data.strm && !ev->_major)
			{
				/* this little swizzle is to ensure the address
					gets updated as the object is a SOMREF,
					this means forcing a communication before releasing the 
					'server' object,
					alternatively the server could only delete SOMREFs after
					a timeout, (say 4 minutes) instead of immediately a server
					connection breaks */

				data.cls=SOMDClientProxy_somdTargetGetClassName(data.strm,ev);
			}
		}
		else
		{
			data.strm=SOMDServer_somdCreateObj(data.server,ev,className,hint);
		}
	}

	if (!ev->_major)
	{
		result=data.strm;
		data.strm=NULL;

		if (!result)
		{
			RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,NO);
		}
	}

	RHBOPT_cleanup_pop();

	return result;
}

struct ImplRepository_localize_group
{
	_IDL_SEQUENCE_ImplementationDef *group;
};

RHBOPT_cleanup_begin(ImplRepository_localize_group_cleanup,pv)

	struct ImplRepository_localize_group *data=pv;

	if (data->group)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		SOMD_FreeType(&ev,data->group,somdTC_sequence_SOMObject);
		SOM_UninitEnvironment(&ev);
	}

RHBOPT_cleanup_end

static void ImplRepository_localize_group(
		ImplRepository SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_ImplementationDef *group)
{
/*	ORBStatus status=0; */
	struct ImplRepository_localize_group data={NULL};

	RHBOPT_cleanup_push(ImplRepository_localize_group_cleanup,&data);

	data.group=group;

	/* status= */ ImplRepository_ensure_local(somSelf,ev,group);

	if (!ev->_major)
	{
		data.group=NULL;
	}

	RHBOPT_cleanup_pop();
}

