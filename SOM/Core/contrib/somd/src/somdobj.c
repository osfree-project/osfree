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

#ifndef SOM_Module_somdobj_Source
#	define SOM_Module_somdobj_Source
#endif

#define SOMDObject_Class_Source

#include "somdobj.ih"

#ifdef SOMDObject_parent_SOMRefObject_get_interface
#	ifndef SOMRefObject_get_interface
#		error SOMRefObject_get_interface missing
#	endif
#	ifndef SOMObject_get_interface
#		error SOMObject_get_interface missing
#	endif
#endif

#ifdef SOMDObject_parent_SOMObject_get_interface
#	ifndef SOMObject_get_interface
#		error SOMObject_get_interface missing
#	endif
#endif

SOM_Scope ImplementationDef SOMSTAR  SOMLINK somdobj_get_implementation(
		SOMDObject SOMSTAR somSelf, 
		Environment *ev)
{
    SOMDObjectData *somThis=SOMDObjectGetData(somSelf);
	char *id=somdExtractUUID(ev,&somThis->object.object_key);
	ImplementationDef SOMSTAR def=RHBImplementationDef_get_wrapper(somThis->object.impl,ev,id);
	if (id) SOMFree(id);
	return def;
}

struct somdobj_get_interface
{
	char *type;
};

RHBOPT_cleanup_begin(somdobj_get_interface_cleanup,pv)

	struct somdobj_get_interface *data=pv;
	if (data->type) SOMFree(data->type);

RHBOPT_cleanup_end

SOM_Scope InterfaceDef SOMSTAR  SOMLINK somdobj_get_interface(
		SOMDObject SOMSTAR somSelf, 
		Environment *ev)
{
    SOMDObjectData *somThis = SOMDObjectGetData(somSelf);
	struct somdobj_get_interface data={NULL};
	InterfaceDef SOMSTAR RHBOPT_volatile def=NULL;

	RHBOPT_cleanup_push(somdobj_get_interface_cleanup,&data);

	RHBSOM_Trace("Get interface")

	if (somThis->object.iface)
	{
		def=ORB_lookup_id(SOMD_ORBObject,ev,NULL,somThis->object.iface->id);
	}
	else
	{
		RHBSOM_Trace("interface not as separate entity")

		data.type=SOMDObject__get_type_id(somSelf,ev);

		if (data.type)
		{
			def=ORB_lookup_id(SOMD_ORBObject,ev,NULL,data.type);
		}
	}

	RHBOPT_cleanup_pop();

    return def;
}

SOM_Scope boolean  SOMLINK somdobj_is_nil(SOMDObject SOMSTAR somSelf, 
                                           Environment *ev)
{
	RHBOPT_unused(ev)

	if (somSelf)
	{
		SOMDObjectData *somThis = SOMDObjectGetData(somSelf);

		if (somThis->object.object_key._length==0) return 1;

		return 0;

/*		boolean invoke("_non_existent") ????*/
	}

    return 1;
}

SOM_Scope boolean  SOMLINK somdobj_is_SOM_ref(SOMDObject SOMSTAR somSelf, 
                                               Environment *ev)
{
	SOMDObjectData *somThis = SOMDObjectGetData(somSelf);

	RHBOPT_unused(ev)

	if (somThis->object.object_key._length < OBJECTKEY_PREFIX_LEN)
	{
		return 0;
	}

	if (somThis->object.object_key._buffer[0]==3)
	{
		if (somThis->object.object_key._length==OBJECTKEY_PREFIX_LEN)
		{
			return 1;
		}

		if (somThis->object.object_key._buffer[
				OBJECTKEY_PREFIX_LEN-1]==2)
		{
			return 1;
		}
	}

    return 0;
}

SOM_Scope boolean  SOMLINK somdobj_is_constant(SOMDObject SOMSTAR somSelf, 
                                                Environment *ev)
{
	SOMDObjectData *somThis = SOMDObjectGetData(somSelf);

	RHBOPT_unused(ev)

	if (somThis->object.object_key._length > 6)
	{
		if (0==memcmp(somThis->object.object_key._buffer,
				"SOM|0|",6))
		{
			return 1;
		}
	}

    return 0;
}

SOM_Scope boolean  SOMLINK somdobj_is_proxy(SOMDObject SOMSTAR somSelf, 
                                             Environment *ev)
{
	RHBOPT_unused(ev)
	RHBOPT_unused(somSelf)

    return 0;
}

SOM_Scope SOMDObject SOMSTAR  SOMLINK somdobj_duplicate(SOMDObject SOMSTAR somSelf, 
                                                  Environment *ev)
{
	RHBOPT_unused(ev)

#ifdef SOMDObject_somDuplicateReference
	return SOMDObject_somDuplicateReference(somSelf);
#else
    SOMDObjectData *somThis = SOMDObjectGetData(somSelf);

	RHBGIOPORB_atomic_inc(&somThis->object.lUsage);

    return somSelf;
#endif
}

SOM_Scope void  SOMLINK somdobj_release(SOMDObject SOMSTAR somSelf, 
                                         Environment *ev)
{
#ifdef SOMDObject_somRelease
	RHBOPT_unused(ev)
	SOMDObject_somRelease(somSelf);
#else
    SOMDObjectData *somThis = SOMDObjectGetData(somSelf);

	if (RHBGIOPORB_atomic_dec(&somThis->object.lUsage) <= 0)
	{
		SOMObject_somFree(somSelf);
	}
#endif
}

static void RHBContext_add_string(_IDL_SEQUENCE_string *seq,char *p)
{
	if (seq->_length==seq->_maximum)
	{
		char **_buffer;
		unsigned int i=0;

		seq->_maximum+=4;
		_buffer=SOMMalloc(sizeof(*_buffer)*seq->_maximum);

		while (i < seq->_length)
		{
			_buffer[i]=seq->_buffer[i];
			i++;
		}
		if (seq->_buffer) SOMFree(seq->_buffer);
		seq->_buffer=_buffer;
	}

	seq->_buffer[seq->_length]=somd_dupl_string(p);
	seq->_length++;
}

static void RHBContext_merge(_IDL_SEQUENCE_string *seq,
							 Context SOMSTAR somSelf,
							 Environment *ev,char *name,Flags flags)
{
	NVList SOMSTAR list=NULL;

	Context_get_values(somSelf,ev,0,flags,name,&list);

	if (list && !ev->_major)
	{
		long count=0;

		NVList_get_count(list,ev,&count);

		if (count && !ev->_major)
		{
			long i=0;

			while (i < count)
			{
				NamedValue nv=RHBSOMD_Init_NamedValue;
				char *p=NULL;

				NVList_get_item(list,ev,i,
					&nv.name,
					&nv.argument._type,
					&nv.argument._value,
					&nv.len,
					&nv.arg_modes);

				RHBContext_add_string(seq,nv.name);
				p=RHBCDR_cast_any_to_string(ev,&nv.argument);
				RHBContext_add_string(seq,p);
				i++;
			}
		}

		NVList_somFree(list);
	}
}

SOM_Scope ORBStatus  SOMLINK somdobj_create_request(SOMDObject SOMSTAR somSelf, 
                                                    Environment *ev, 
                                                    Context SOMSTAR ctx, 
                                                    Identifier operation, 
                                                    NVList SOMSTAR arg_list, 
                                                    NamedValue* result, 
                                                    Request SOMSTAR* request, 
                                                    Flags req_flags)
{
    SOMDObjectData *somThis = SOMDObjectGetData(somSelf);
	RHBRequest *req=NULL;
	_IDL_SEQUENCE_string context={0,0,NULL};

	if (ev->_major) return SOMDERROR_BadEnvironment;

	if (ctx)
	{
		RHBContext_merge(&context,ctx,ev,"*",CTX_RESTRICT_SCOPE);
	}

	RHBObject_create_request(
		&somThis->object,
		somSelf,
		ev,
		ctx ? &context : 0,
		operation,
		arg_list,
		result,
		&req,
		req_flags);

	if (req)
	{
		Request SOMSTAR r=RHBORB_newRequest(somThis->object.impl->orb);

		Request__set_c_request(r,ev,req);

		(*request)=r;

		return 0;
	}

	while (context._length--)
	{
		char *p;
		p=context._buffer[context._length];
		if (p) SOMFree(p);
	}

	if (context._buffer)
	{
		SOMFree(context._buffer);
	}

	*request=0;

	return 0xffffffff;
}

struct somdobj_create_request_args
{
	InterfaceDef SOMSTAR iface;
	_IDL_SEQUENCE_SOMObject seq;
	somId id_lookup_name;
	any desc_value;
	_IDL_SEQUENCE_somModifier mods;
	unsigned short describe_flags;
};

RHBOPT_cleanup_begin(somdobj_create_request_args_cleanup,pv)

struct somdobj_create_request_args *data=pv;
Environment ev;

	SOM_InitEnvironment(&ev);

	SOMD_FreeType(&ev,&data->seq,somdTC_sequence_SOMObject);
	SOMD_FreeType(&ev,&data->iface,TC_Object);

	if (data->id_lookup_name) SOMFree(data->id_lookup_name);

	SOMD_FreeType(&ev,&data->desc_value,TC_any);

	SOMD_FreeType(&ev,&data->mods,somdTC_sequence_somModifier);

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

SOM_Scope ORBStatus  SOMLINK somdobj_create_request_args(SOMDObject SOMSTAR somSelf, 
                                                          Environment *ev, 
                                                         Identifier operation, 
                                                         NVList SOMSTAR* arg_list, 
                                                         NamedValue* result)
{
	struct somdobj_create_request_args data={NULL,{0,0,NULL},
											NULL,{NULL,NULL},
											{0,0,NULL},0};
	ORBStatus RHBOPT_volatile status=SOMDERROR_BadParm;

	*arg_list=NULL;

	result->name=NULL;
	result->arg_modes=RESULT_OUT;
	result->len=0;
	result->argument._type=TC_void;
	result->argument._value=NULL;

	RHBOPT_cleanup_push(somdobj_create_request_args_cleanup,&data);

	data.iface=SOMDObject_get_interface(somSelf,ev);

	if (data.iface && !ev->_major)
	{
/*		data.id_lookup_name=somIdFromString("lookup_name");

		va_SOMObject_somDispatch(data.iface,(somToken *)&data.seq,data.id_lookup_name,
				data.iface,ev,operation,(long)-1,
				"OperationDef",
				0);
*/
		data.seq=ORB_lookup_name(SOMD_ORBObject,ev,
			data.iface,operation,-1,CORBA_dk_Operation,0);

		if (!ev->_major)
		{
			unsigned long i=data.seq._length;

			while (i-- && !ev->_major)
			{
				if (data.seq._buffer[i])
				{
					ORB_contained_describe(SOMD_ORBObject,ev,
							data.seq._buffer[i],
							&data.describe_flags,
							&data.desc_value,
							&data.mods,
							NULL,
							NULL);

					if (!ev->_major)
					{
						*arg_list=ORB_create_list_from_opdesc_value(
								SOMD_ORBObject,ev,
								&data.desc_value,
								&data.mods,
								SOMDObject_is_proxy(somSelf,ev) ?
									CLIENT_SIDE : 0);

						if (!ev->_major)
						{
							any a=RHBCDR_get_named_element(ev,&data.desc_value,"result");

							if (!ev->_major)
							{
								TypeCode tc=RHBCDR_cast_any_to_TypeCode(ev,&a);

								result->argument._type=tcCopy(tc,ev);

								if (ORB_get_somModifier(NULL,ev,
										&data.mods,
										"object_owns_result"))
								{
									result->arg_modes|=OBJ_OWNED;
								}
		
								status=0;
							}
						}

						if (ev->_major && *arg_list)
						{
							NVList SOMSTAR list=*arg_list;
							*arg_list=NULL;
							somReleaseObjectReference(list);
						}
					}

					break;
				}
			}
		}
	}

	RHBOPT_cleanup_pop();

    return status;
}

SOM_Scope void  SOMLINK somdobj_somDumpSelfInt(
		SOMDObject SOMSTAR somSelf, 
		long level)
{
    SOMDObjectData *somThis = SOMDObjectGetData(somSelf);
   
	RHBObject_somDumpSelfInt(&somThis->object,
			somSelf,level);
}

SOM_Scope RHBObject * SOMLINK somdobj__get_c_proxy(
			SOMDObject SOMSTAR somSelf,
			Environment *ev)
{
	RHBOPT_unused(ev)

	return &(SOMDObjectGetData(somSelf)->object);
}

SOM_Scope boolean  SOMLINK somdobj_somDispatch(SOMDObject SOMSTAR somSelf, 
                                                            somToken* retValue, 
                                                            somId methodId, 
                                                            va_list ap)
{
#ifdef SOMDObject_parent_SOMRefObject_somDispatch
	return SOMDObject_parent_SOMRefObject_somDispatch(somSelf,
				retValue,methodId,ap);
#else
	return SOMDObject_parent_SOMObject_somDispatch(somSelf,
				retValue,methodId,ap);
#endif
}

#ifdef SOMDObject_somDestruct
SOM_Scope void SOMLINK somdobj_somDestruct(
	SOMDObject SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SOMDObjectData *somThis;

	SOMDObject_BeginDestructor
	RHBObject_somUninit(&somThis->object);
	SOMDObject_EndDestructor
}
#else
SOM_Scope void SOMLINK somdobj_somUninit(SOMDObject SOMSTAR somSelf)
{
    SOMDObjectData *somThis = SOMDObjectGetData(somSelf);
	RHBObject_somUninit(&somThis->object);

#	ifdef SOMDObject_parent_SOMRefObject_somUninit
	SOMDObject_parent_SOMRefObject_somUninit(somSelf);
#	else
	SOMDObject_parent_SOMObject_somUninit(somSelf);
#	endif
}
#endif

#ifdef SOMObject_somDefaultInit
SOM_Scope void SOMLINK somdobj_somDefaultInit(
	SOMDObject SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SOMDObjectData *somThis;

	SOMDObject_BeginInitializer_somDefaultInit
	SOMDObject_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
	RHBOPT_unused(somThis);
}
#else
SOM_Scope void SOMLINK somdobj_somInit(SOMDObject SOMSTAR somSelf)
{
/*   SOMDObjectData *somThis = SOMDObjectGetData(somSelf);*/

	RHBOPT_unused(somSelf)

#	ifdef SOMDObject_parent_SOMRefObject_somInit
	SOMDObject_parent_SOMRefObject_somInit(somSelf);
#	else
	SOMDObject_parent_SOMObject_somInit(somSelf);
#	endif
}
#endif

SOM_Scope corbastring SOMLINK somdobj__get_type_id(
	SOMDObject SOMSTAR somSelf,
	Environment *ev)
{
    SOMDObjectData *somThis = SOMDObjectGetData(somSelf);

	if (somThis->object.iface)
	{
		return somd_dupl_string(somThis->object.iface->id);
	}

	RHBOPT_unused(ev)

	return NULL;
}

SOM_Scope void SOMLINK somdobj__set_type_id(
	SOMDObject SOMSTAR somSelf,
	Environment *ev,corbastring type_id)
{
    SOMDObjectData *somThis=SOMDObjectGetData(somSelf);
	RHBInterfaceDef *oldIf=NULL;
	RHBORB *orb=NULL;

	RHBORB_guard(somThis->orb)

	if (somThis->object.impl)
	{
		orb=somThis->object.impl->orb;
	}
	
	if(!orb)
	{
		orb=ORB__get_c_orb(SOMD_ORBObject,ev);
	}

	if (orb)
	{
		oldIf=somThis->object.iface;
		somThis->object.iface=RHBORB_get_interface(orb,ev,type_id);
	}

	RHBORB_unguard(somThis->orb)

	if (oldIf) RHBInterfaceDef_Release(oldIf);
}

SOM_Scope SOMObject SOMSTAR SOMLINK somdobj_somPrintSelf(SOMDObject SOMSTAR somSelf)
{
	 dump_somPrintf(("An instance of %s at address %p\n",
			somSelf->mtab->className,
			somSelf));

     return somSelf;
}

RHBOPT_cleanup_begin(somdobj_somdMarshal_cleanup,pv)

IOP_IOR *ior=pv;

	Environment ev2;
	SOM_InitEnvironment(&ev2);
	SOMD_FreeType(&ev2,ior,somdTC_IOP_IOR);
	SOM_UninitEnvironment(&ev2);

RHBOPT_cleanup_end

SOM_Scope void SOMLINK somdobj_somdMarshal(
		SOMDObject SOMSTAR somSelf,
		Environment *ev,
		CORBA_DataOutputStream SOMSTAR stream)
{
	IOP_IOR ior={NULL,{0,0,NULL}};
	RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);
	SOMDObjectData *somThis=SOMDObjectGetData(somSelf);
	RHBObject *obj=&somThis->object;

	if (!ev->_major)
	{
		RHBOPT_cleanup_push(somdobj_somdMarshal_cleanup,&ior);

		ior.type_id=SOMDObject__get_type_id(somSelf,ev);

		if (!ev->_major)
		{
			if (obj->impl)
			{
				IIOP_ProfileBody_1_0 body={{1,0},NULL,0,{0,0,NULL}};
				any b={NULL,NULL};

				ior.profiles._length=1;
				ior.profiles._maximum=1;
				ior.profiles._buffer=SOMMalloc(sizeof(ior.profiles._buffer[0]));

				body.port=obj->impl->address.port;
				body.host=obj->impl->address.host;

				body.object_key._length=obj->object_key._length;
				body.object_key._maximum=obj->object_key._maximum;
				body.object_key._buffer=obj->object_key._buffer;

				b._value=&body;
				b._type=somdTC_IIOP_ProfileBody_1_0;

				ior.profiles._buffer[0].profile_data=IOP_Codec_encode_value(orb->iop_codec,ev,&b);
				ior.profiles._buffer[0].tag=IOP_TAG_INTERNET_IOP;
			}

			RHBCDR_marshal(NULL,ev,&ior,somdTC_IOP_IOR,stream);
		}

		RHBOPT_cleanup_pop();
	}
}

SOM_Scope void SOMLINK somdobj__set_somd_flags(
		SOMDObject SOMSTAR somSelf,
		Environment *ev,
		unsigned short flags)
{
}

SOM_Scope void SOMLINK somdobj__set_somd_rid(
		SOMDObject SOMSTAR somSelf,
		Environment *ev,
		somToken rid)
{
}

SOM_Scope void SOMLINK somdobj__set_somd_id(
		SOMDObject SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_octet *id)
{
}

SOM_Scope void SOMLINK somdobj__set_somd_impl(
		SOMDObject SOMSTAR somSelf,
		Environment *ev,
		somToken impl)
{
}

SOM_Scope somToken SOMLINK somdobj__get_somd_impl(
		SOMDObject SOMSTAR somSelf,
		Environment *ev)
{
	return NULL;
}

SOM_Scope somToken SOMLINK somdobj__get_somd_rid(
		SOMDObject SOMSTAR somSelf,
		Environment *ev)
{
	return NULL;
}

SOM_Scope somToken SOMLINK somdobj__get_somd_tag(
		SOMDObject SOMSTAR somSelf,
		Environment *ev)
{
	return NULL;
}

SOM_Scope unsigned short SOMLINK somdobj__get_somd_flags(
		SOMDObject SOMSTAR somSelf,
		Environment *ev)
{
	return 0;
}

SOM_Scope _IDL_SEQUENCE_octet SOMLINK somdobj__get_somd_id(
		SOMDObject SOMSTAR somSelf,
		Environment *ev)
{
	SOMDObjectData *somThis=SOMDObjectGetData(somSelf);

	return somThis->object.object_key;
}

SOM_Scope void SOMLINK somdobj_clsInit(SOMClass SOMSTAR somSelf)
{
#ifdef SOMDObject_parent_SOMRefObject_duplicate
	SOMObjectClassDataStructure *socd=&(SOMObjectClassData);
	SOMDObjectClassDataStructure *docd=&(SOMDObjectClassData);
#define MIGRATE_METHOD(n)	docd->n=socd->n; RHBOPT_ASSERT(docd->n)

		MIGRATE_METHOD(duplicate);
		MIGRATE_METHOD(release);
		MIGRATE_METHOD(is_proxy);
		MIGRATE_METHOD(get_implementation);
		MIGRATE_METHOD(get_interface);
		MIGRATE_METHOD(create_request);
		MIGRATE_METHOD(create_request_args);
#else
#	if ((SOMObject_MajorVersion > 1)||(SOMObject_MinorVersion >= 7))
		#error what happened here?
#	endif
#endif
}
