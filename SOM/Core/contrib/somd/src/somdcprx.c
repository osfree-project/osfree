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

#ifndef SOM_Module_somdcprx_Source
#define SOM_Module_somdcprx_Source
#endif
#define SOMDClientProxy_Class_Source

#include "somdcprx.ih"

struct somdcprx_somdTargetFree
{
	Request SOMSTAR request;
};

RHBOPT_cleanup_begin(somdcprx_somdTargetFree_cleanup,pv)

struct somdcprx_somdTargetFree *data=pv;

	if (data->request) Request_somRelease(data->request);

RHBOPT_cleanup_end

SOM_Scope void  SOMLINK somdcprx_somdTargetFree(
		SOMDClientProxy SOMSTAR somSelf, 
		Environment *ev)
{
	struct somdcprx_somdTargetFree data={NULL};
/*	NamedValue result=RHBSOMD_Init_NamedValue;

	result.argument._type=TC_void;*/

	/* should really get the matching SOMDServer for this
		and then send it a somdDeleteObj? or should
		that wrapping happen at the server end, ie, detects "somFree"
		and turns it into somdDeleteObj?... */

	RHBOPT_cleanup_push(somdcprx_somdTargetFree_cleanup,&data);

	if (!SOMDClientProxy_create_request(somSelf,ev,
			NULL,
			"somFree",
			NULL,
			NULL /*&result*/,
			&data.request,
			0))
	{
		Request_invoke(data.request,ev,INV_NO_RESPONSE);
	}

	RHBOPT_cleanup_pop();
}

struct somdcprx_somdTargetGetClass
{
	Request SOMSTAR request;
	TypeCode tc;
};

RHBOPT_cleanup_begin(somdcprx_somdTargetGetClass_cleanup,pv)

	struct somdcprx_somdTargetGetClass *data=pv;

	if (data->request) Request_somRelease(data->request);
	if (data->tc)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		TypeCode_free(data->tc,&ev);
		SOM_UninitEnvironment(&ev);
	}

RHBOPT_cleanup_end

SOM_Scope SOMClass SOMSTAR  SOMLINK somdcprx_somdTargetGetClass(
		SOMDClientProxy SOMSTAR somSelf, 
		Environment *ev)
{
	SOMClass SOMSTAR cls=NULL;
	struct somdcprx_somdTargetGetClass data={NULL,NULL};
	NamedValue result=RHBSOMD_Init_NamedValue;

	RHBOPT_cleanup_push(somdcprx_somdTargetGetClass_cleanup,&data);

	data.tc=TypeCodeNew(tk_objref,"::SOMClass");

	result.argument._type=data.tc;
	result.argument._value=&cls;

	SOMDClientProxy_create_request(somSelf,ev,
			NULL,
			"somGetClass",
			NULL,
			&result,
			&data.request,
			0);

	if (data.request && !ev->_major)
	{
		Request_invoke(data.request,ev,0);
	}

	RHBOPT_cleanup_pop();

	return cls;
}

struct somdcprx_somdTargetGetClassName
{
	Request SOMSTAR request;
};

RHBOPT_cleanup_begin(somdcprx_somdTargetGetClassName_cleanup,pv)

struct somdcprx_somdTargetGetClassName *data=pv;

	if (data->request) Request_somRelease(data->request);

RHBOPT_cleanup_end

SOM_Scope corbastring  SOMLINK somdcprx_somdTargetGetClassName(
		SOMDClientProxy SOMSTAR somSelf, 
		Environment *ev)
{
	char *clsName=NULL;
	struct somdcprx_somdTargetGetClassName data={NULL};
	NamedValue result=RHBSOMD_Init_NamedValue;

	result.argument._type=TC_string;
	result.argument._value=&clsName;

	RHBOPT_cleanup_push(somdcprx_somdTargetGetClassName_cleanup,&data);

	SOMDClientProxy_create_request(somSelf,ev,
			NULL,
			"somGetClassName",
			NULL,
			&result,
			&data.request,
			0);

	if (data.request && !ev->_major)
	{
		Request_invoke(data.request,ev,0);
	}

	RHBOPT_cleanup_pop();

	return clsName;
}

SOM_Scope void  SOMLINK somdcprx_somdProxyFree(
	SOMDClientProxy SOMSTAR somSelf, 
	Environment *ev)
{
	somReleaseObjectReference(somSelf);
	RHBOPT_unused(ev)
}

SOM_Scope SOMClass SOMSTAR  SOMLINK somdcprx_somdProxyGetClass(
		SOMDClientProxy SOMSTAR somSelf, 
		Environment *ev)
{
	RHBOPT_unused(ev)
    return somSelf->mtab->classObject;
}

SOM_Scope corbastring  SOMLINK somdcprx_somdProxyGetClassName(
	SOMDClientProxy SOMSTAR somSelf, 
	Environment *ev)
{
	RHBOPT_unused(ev)
    return somSelf->mtab->className;
}

SOM_Scope void  SOMLINK somdcprx_somdReleaseResources(
		SOMDClientProxy SOMSTAR somSelf, 
		Environment *ev)
{
	/* we should maintain a list of all
		object owned arguments and results,
		then free them here */

	RHBOPT_unused(ev)
	RHBOPT_unused(somSelf)
}

SOM_Scope boolean  SOMLINK somdcprx_is_proxy(
		SOMDClientProxy SOMSTAR somSelf, 
		Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

    return 1;
}

struct somdcprx_somDispatch
{
	boolean return_array;
	SOMObject SOMSTAR proxyTarget;
	Request SOMSTAR request;
	char *operation;
	NVList SOMSTAR nvlist;
	union values
	{
		boolean value_boolean;
		char value_char;
		octet value_octet;
		short value_short;
		unsigned short value_unsigned_short;
		long value_long;
		unsigned long value_unsigned_long;
		float value_float;
		double value_double;
		void *value_pointer;
	} *values;
	NamedValue result;
	Environment evLocal;
};

RHBOPT_cleanup_begin(somdcprx_somDispatch_cleanup,pv)

struct somdcprx_somDispatch *data=pv;
Environment ev;

	SOM_InitEnvironment(&ev);

	if (data->values) SOMFree(data->values);

	if (data->proxyTarget)
	{
#ifdef SOMObject_somRelease
		somReleaseObjectReference(data->proxyTarget);
#else
		if (SOMObject_somIsA(data->proxyTarget,_SOMRefObject))
		{
			SOMRefObject_somRelease(data->proxyTarget);
		}
#endif
	}

	if (data->nvlist)
	{
		somReleaseObjectReference(data->nvlist);
	}

	if (data->result.argument._type)
	{
		TypeCode_free(data->result.argument._type,&ev);
	}

#ifdef USE_APPLE_SOM
	if (data->operation)
	{
		SOMFree(data->operation);
	}
#endif

	if (data->request)
	{
		somReleaseObjectReference(data->request);
	}

	if (data->return_array && data->result.argument._value)
	{
		SOMFree(data->result.argument._value);
	}

	SOM_UninitEnvironment(&ev);
	SOM_UninitEnvironment(&data->evLocal);

RHBOPT_cleanup_end

SOM_Scope boolean SOMLINK somdcprx_somDispatch(SOMDClientProxy SOMSTAR somSelf, 
                                                somToken* retValue_arg, 
                                                somId methodId, 
#if (defined(HAVE_VA_COPY_VOLATILE) || defined(HAVE_VA_LIST_ASSIGN)) && defined(HAVE_VA_ARG_VOLATILE)
                                                va_list ap_arg
#else
												va_list ap
#endif
												)
{
	RHBObject *c_proxy=SOMDObject__get_c_proxy(somSelf,0);
	struct somdcprx_somDispatch data={0,
			NULL,NULL,NULL,NULL,NULL,
			RHBSOMD_Init_NamedValue};
	boolean RHBOPT_volatile b=0;
	somToken * RHBOPT_volatile retValue=retValue_arg;

	/* this is to get around
		making ap volatile */

#ifdef HAVE_VA_ARG_VOLATILE
#	ifdef HAVE_VA_COPY_VOLATILE
			va_list RHBOPT_volatile ap;
			va_copy(ap,ap_arg);
#	else
#		ifdef HAVE_VA_LIST_ASSIGN
			va_list RHBOPT_volatile ap=ap_arg;
#		endif
#	endif
#endif

	SOM_InitEnvironment(&data.evLocal);

	RHBOPT_cleanup_push(somdcprx_somDispatch_cleanup,&data);

	if (c_proxy->impl)
	{
		if (SOMClass_somGetMethodToken(c_proxy->impl->orb->somClass_SOMDClientProxy_ref,methodId))
		{
			b=SOMDClientProxy_parent_SOMDObject_somDispatch(somSelf,retValue,methodId,ap);
		}
		else
		{
			if (c_proxy->impl->connection.is_listener)
			{
				data.proxyTarget=SOMDClientProxy__get_sommProxyTarget(somSelf);

				if (data.proxyTarget)
				{
					if (data.proxyTarget != somSelf)
					{
						somvalistSetTarget(ap,data.proxyTarget);

						b=SOMObject_somDispatch(data.proxyTarget,retValue,methodId,ap);
					}
				}
			}
			else
			{
#if 1
#ifdef USE_APPLE_SOM
				data.operation=somMakeStringFromId(methodId);
#else
				data.operation=somStringFromId(methodId);
#endif

				SOMDClientProxy_create_request_args(somSelf,
							&data.evLocal,
							data.operation,
							&data.nvlist,
							&data.result);

				if (!data.evLocal._major)
				{
					Context SOMSTAR ctx=NULL;
					Environment *ev=&data.evLocal;
					boolean oneway=0;
					SOMDObject SOMSTAR target=va_arg(ap,SOMDObject SOMSTAR);

					RHBOPT_ASSERT(target==somSelf)

					if (data.result.arg_modes & INV_NO_RESPONSE)
					{
						oneway=1;
					}

					if (data.result.argument._type)
					{
						switch (TypeCode_kind(data.result.argument._type,ev))
						{
						case tk_void:
						case tk_null:
							break;
						case tk_array:
							/* array is a special case for returns */
							data.return_array=1;
							data.result.len=TypeCode_size(data.result.argument._type,ev);
							if (data.result.len)
							{
								data.result.argument._value=SOMMalloc(data.result.len);
								memset(data.result.argument._value,0,data.result.len);
							}
							break;
						default:
							data.result.argument._value=retValue;
							break;
						}
					}

					ev=va_arg(ap,Environment *);

					if (data.result.arg_modes & CTX_IN_MSG)
					{
						ctx=va_arg(ap,Context SOMSTAR);
					}

					if (data.nvlist)
					{
						long count=0;

						NVList_get_count(data.nvlist,ev,&count);

						if (count)
						{
							long arg_modes=oneway ? IN_COPY_VALUE : 0;
							long i=0;

							data.values=SOMMalloc(count*sizeof(data.values[0]));

							while ((i < count)&&(!ev->_major))
							{
								NamedValue param=RHBSOMD_Init_NamedValue;

								NVList_get_item(data.nvlist,ev,i,
									&param.name,
									&param.argument._type,
									&param.argument._value,
									&param.len,
									&param.arg_modes);

								param.arg_modes|=arg_modes;

								switch (param.arg_modes &
									(ARG_IN|ARG_OUT|ARG_INOUT))
								{
								case ARG_IN:
									switch (TypeCode_kind(param.argument._type,ev))
									{
#define case_type(x,y,z,c)  case tk_##x: \
									data.values[i].value_##y=(c)va_arg(ap,z); \
									param.argument._value=&(data.values[i].value_##y); \
									break;
#define case_type2(a,b)	  case_type(a,a,b,a)
									case_type2(octet,int)
									case_type2(char,int)
									case_type2(boolean,int)
									case_type2(short,int)
									case_type(ushort,unsigned_short,int,unsigned short)
									case_type2(long,long)
									case_type(ulong,unsigned_long,unsigned long,unsigned long)
									case_type2(double,double)
									case_type(float,float,double,float)
									case tk_pointer:
									case tk_objref:
									case tk_string:
									case tk_TypeCode:
									case tk_Principal:
										data.values[i].value_pointer=va_arg(ap,void *);
										param.argument._value=&(data.values[i].value_pointer);
										break;
									case tk_union:
									case tk_struct:
									case tk_sequence:
									case tk_array:
									case tk_any:
										param.argument._value=va_arg(ap,void *);
										break;
									default:
										RHBOPT_throw_StExcep(ev,BAD_TYPECODE,DispatchError,NO);
										break;
									}
									break;
								case ARG_OUT:
								case ARG_INOUT:
									if (oneway)
									{
										RHBOPT_throw_StExcep(ev,BAD_TYPECODE,DispatchError,NO);
									}
									else
									{
										param.argument._value=va_arg(ap,void *);
									}
									break;
								}

								if (ev->_major)
								{
									break;
								}
								else
								{
									NVList_set_item(data.nvlist,ev,i,
												param.name,
												param.argument._type,
												param.argument._value,
												param.len,
												param.arg_modes);
								}

								i++;
							}
						}
					}

					if (!ev->_major)
					{
						SOMDClientProxy_create_request(target,ev,
							ctx,
							data.operation,
							data.nvlist,
							&data.result,
							&data.request,0);

						if (!ev->_major)
						{
							if (oneway)
							{
								Request_invoke(data.request,ev,INV_NO_RESPONSE);
							}
							else
							{
								Request_invoke(data.request,ev,0);
							}

							b=1;

							if (data.return_array && !ev->_major)
							{
								/* arrays are returned as a pointer to the first slice...,
									but we are keeping the 'any' type format sacred...
								*/
								retValue[0]=data.result.argument._value;
								data.result.argument._value=NULL;

							}
						}
					}
				}
#else
				b=RHBObject_somDispatch(c_proxy,somSelf,retValue,methodId,ap);
#endif
			}
		}
	}

	RHBOPT_cleanup_pop();

#ifdef HAVE_VA_COPY_VOLATILE
	va_end(ap);
#endif

	return b;
}

#ifdef SOMDClientProxy_parent_SOMDObject_somGetClass
SOM_Scope SOMClass SOMSTAR SOMLINK somdcprx_somGetClass(
	SOMDClientProxy SOMSTAR somSelf)
{
	Environment ev;
	SOMClass SOMSTAR cls;
	SOM_InitEnvironment(&ev);
	cls=somdcprx_somdTargetGetClass(somSelf,&ev);
	SOM_UninitEnvironment(&ev);
	return cls;
}
#endif

#ifdef SOMDClientProxy_parent_SOMDObject_somGetClassName
SOM_Scope corbastring  SOMLINK somdcprx_somGetClassName(
	SOMDClientProxy SOMSTAR somSelf)
{
	Environment ev;
	char *cls;
	SOM_InitEnvironment(&ev);
	cls=somdcprx_somdTargetGetClassName(somSelf,&ev);
	SOM_UninitEnvironment(&ev);
	return cls;
}
#endif

#ifdef SOMDClientProxy_somDefaultInit
SOM_Scope void SOMLINK somdcprx_somDefaultInit(
	SOMDClientProxy SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	SOMDClientProxyData *somThis; */

	SOMDClientProxy_BeginInitializer_somDefaultInit
	SOMDClientProxy_Init_SOMMProxyForObject_somDefaultInit(somSelf,ctrl);
	SOMDClientProxy_Init_SOMDObject_somDefaultInit(somSelf,ctrl);
}
#else
SOM_Scope void SOMLINK cntxt_somInit(SOMDClientProxy SOMSTAR somSelf)
{
	SOMDClientProxyData *somThis=ContextGetData(somSelf);

	SOMDClientProxy_parent_SOMMProxyForObject_somInit(somSelf);
	SOMDClientProxy_parent_SOMDObject_somInit(somSelf);
}
#endif


#ifdef SOMDClientProxy_somDestruct
SOM_Scope void SOMLINK somdcprx_somDestruct(
	SOMDClientProxy SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	SOMDClientProxyData *somThis;  */

	SOMDClientProxy_BeginDestructor
	SOMDClientProxy_EndDestructor
}
#else
SOM_Scope void SOMLINK somdcprx_somUninit(
		SOMDClientProxy SOMSTAR somSelf)
{
	SOMDClientProxy_parent_SOMDObject_somUninit(somSelf);
	SOMDClientProxy_parent_SOMMProxyForObject_somUninit(somSelf);
}
#endif

struct somdcprx_get_interface
{
	Request SOMSTAR request;
};

RHBOPT_cleanup_begin(somdcrpx_get_interface_cleanup,pv)

struct somdcprx_get_interface *data=pv;

	if (data->request) Request_somRelease(data->request);

RHBOPT_cleanup_end

SOM_Scope InterfaceDef SOMSTAR  SOMLINK somdcprx_get_interface(
		SOMDClientProxy SOMSTAR somSelf, 
		Environment *ev)
{
	InterfaceDef SOMSTAR iface=NULL;

	if (ev && !ev->_major) 
	{
		NamedValue result=RHBSOMD_Init_NamedValue;
		struct somdcprx_get_interface data={NULL};

		RHBOPT_cleanup_push(somdcrpx_get_interface_cleanup,&data);

		result.argument._type=somdTC_CORBA_InterfaceDef;
		result.argument._value=&iface;

		SOMDClientProxy_create_request(
				somSelf,
				ev,
				NULL,
				"_interface",
				NULL,
				&result,
				&data.request,
				0);

		if (data.request && !ev->_major) 
		{
			Request_invoke(data.request,ev,0);
		}

		RHBOPT_cleanup_pop();
	}

	return iface;
}

struct somdcprx__get_sommProxyTarget
{
	SOMDServer SOMSTAR server;
};

RHBOPT_cleanup_begin(somdcprx__get_sommProxyTarget_cleanup,pv)

struct somdcprx__get_sommProxyTarget *data=pv;

	if (data->server) SOMDServer_somRelease(data->server);

RHBOPT_cleanup_end

SOM_Scope SOMObject SOMSTAR SOMLINK somdcprx__get_sommProxyTarget(
	SOMDClientProxy SOMSTAR somSelf)
{
	SOMObject SOMSTAR RHBOPT_volatile o;
	RHBObject *c_proxy;
	Environment ev;
	SOM_InitEnvironment(&ev);

	o=SOMDClientProxy_parent_SOMMProxyForObject__get_sommProxyTarget(somSelf);

	if (o) return o;

	c_proxy=SOMDObject__get_c_proxy(somSelf,&ev);

	if (c_proxy->impl->connection.is_listener)
	{
		struct somdcprx__get_sommProxyTarget data={NULL};

		RHBOPT_cleanup_push(somdcprx__get_sommProxyTarget_cleanup,&data);
		
		data.server=RHBImplementationDef_acquire_somdServer(c_proxy->impl,&ev);

		if (data.server && !ev._major)
		{
			o=SOMDServer_somdSOMObjFromRef(data.server,&ev,somSelf);
		}

		if (ev._major) o=NULL;

		RHBOPT_cleanup_pop();
	}

	SOM_UninitEnvironment(&ev);

	return o;
}

SOM_Scope void SOMLINK somdcprx_somDumpSelfInt(SOMDClientProxy SOMSTAR somSelf,long level)
{
	SOMDClientProxy_parent_SOMDObject_somDumpSelfInt(somSelf,level);
}

struct somdcprx_get_implementation
{
	Request SOMSTAR request;
};

RHBOPT_cleanup_begin(somdcprx_get_implementation_cleanup,pv)

struct somdcprx_get_implementation *data=pv;

	if (data->request) Request_somRelease(data->request);

RHBOPT_cleanup_end

SOM_Scope ImplementationDef SOMSTAR SOMLINK somdcprx_get_implementation(
		SOMDClientProxy SOMSTAR somSelf,
		Environment *ev)
{
	ImplementationDef SOMSTAR impl=NULL;
	NamedValue result=RHBSOMD_Init_NamedValue;
	RHBObject *object=SOMDObject__get_c_proxy(somSelf,0);

	if (object && object->impl && !ev->_major)
	{
		if (object->impl->connection.is_listener)
		{
			impl=SOMDClientProxy_parent_SOMDObject_get_implementation(somSelf,ev);
		}
		else
		{
			struct somdcprx_get_implementation data={NULL};

			RHBOPT_cleanup_push(somdcprx_get_implementation_cleanup,&data);

			result.argument._type=TC_Object;
			result.argument._value=&impl;

			SOMDClientProxy_create_request(
					somSelf,
					ev,
					NULL,
					"_implementation",
					NULL,
					&result,
					&data.request,
					0);

			if (data.request && !ev->_major) 
			{
				Request_invoke(data.request,ev,0);
			}

			RHBOPT_cleanup_pop();
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
	}


	return impl;
}

struct somdcprx_create_request_args
{
	somId	idMethod,
			idDescriptor,
			idGetSomModifiers,
			idLookupName;
	char *id;
	InterfaceDef SOMSTAR iface;
	RHBContained *contained;
	NVList SOMSTAR arg_list;
	_IDL_SEQUENCE_SOMObject seq;
	any desc_value;
	_IDL_SEQUENCE_somModifier mods;
	unsigned short describe_flags;
};

RHBOPT_cleanup_begin(somdcprx_create_request_args_cleanup,pv)

struct somdcprx_create_request_args *data=pv;
Environment ev;

	SOM_InitEnvironment(&ev);

	SOMD_FreeType(&ev,&data->seq,somdTC_sequence_SOMObject);
	SOMD_FreeType(&ev,&data->iface,TC_Object);

	if (data->idLookupName) SOMFree(data->idLookupName);
	if (data->idGetSomModifiers) SOMFree(data->idGetSomModifiers);
	if (data->idMethod) SOMFree(data->idMethod);
	if (data->id) SOMFree(data->id);

#ifdef USE_APPLE_SOM
	if (data->idDescriptor) SOMFree(data->idDescriptor);
#endif

	SOMD_FreeType(&ev,&data->desc_value,TC_any);

	SOMD_FreeType(&ev,&data->mods,somdTC_sequence_somModifier);

	if (data->arg_list)
	{
		somReleaseObjectReference(data->arg_list);
	}

	if (data->contained)
	{
		RHBContained_Release(data->contained);
	}

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

SOM_Scope ORBStatus  SOMLINK somdcprx_create_request_args(SOMDObject SOMSTAR somSelf, 
                                                          Environment *ev, 
                                                         Identifier operation, 
                                                         NVList SOMSTAR* arg_list, 
                                                         NamedValue* result)
{
	struct somdcprx_create_request_args data={NULL,NULL,NULL,NULL,
											NULL,NULL,NULL,NULL,
											{0,0,NULL},
											{NULL,NULL},
											{0,0,NULL},0};
	ORBStatus RHBOPT_volatile status=SOMDERROR_BadParm;
	RHBORB *c_orb=ORB__get_c_orb(SOMD_ORBObject,ev);
	any * RHBOPT_volatile desc_value=NULL;
	_IDL_SEQUENCE_somModifier * RHBOPT_volatile mods=NULL;

	*arg_list=NULL;

	result->name=NULL;
	result->arg_modes=CLIENT_SIDE;
	result->len=0;
	result->argument._type=NULL;
	result->argument._value=NULL;

	data.idMethod=somIdFromString(operation);

#ifdef USE_APPLE_SOM
	if (data.idMethod)
	{
		somKernelId kid=SOMClass_somGetMethodDescriptor(somSelf->mtab->classObject,data.idMethod);

		if (kid)
		{
			data.idDescriptor=somConvertAndFreeKernelId(kid);
		}
	}
#else
	data.idDescriptor=SOMClass_somGetMethodDescriptor(somSelf->mtab->classObject,data.idMethod);

	if (data.idDescriptor)
	{
		debug_somPrintf(("%s\n",somStringFromId(data.idDescriptor)));
	}
#endif

	if (data.idDescriptor)
	{
		data.id=RHBORB_leading_colons(
					c_orb,
					somMakeStringFromId(data.idDescriptor));

		data.contained=RHBORB_get_contained(c_orb,ev,data.id);

		if (data.contained && !ev->_major)
		{
			desc_value=&data.contained->desc_value;
			mods=&data.contained->modifiers;
		}
	}

	RHBOPT_cleanup_push(somdcprx_create_request_args_cleanup,&data);

	if ((!data.contained)&&(!ev->_major))
	{
		data.iface=SOMDObject_get_interface(somSelf,ev);

		if (data.iface && !ev->_major)
		{
/*			data.idLookupName=somIdFromString("lookup_name");

			va_SOMObject_somDispatch(data.iface,
					(somToken *)&data.seq,
					data.idLookupName,
					data.iface,ev,operation,(long)-1,
					"OperationDef",
					0);
*/
			data.seq=ORB_lookup_name(SOMD_ORBObject,ev,
				data.iface,operation,-1,CORBA_dk_Operation,0);

			RHBOPT_ASSERT((ev->_major!=NO_EXCEPTION)||(data.seq._length));
		}

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

					if (ev->_major==NO_EXCEPTION)
					{
						mods=&data.mods;
						desc_value=&data.desc_value;
					}

					break;
				}
			}
		}
	}

	if ((ev->_major==NO_EXCEPTION) && (!desc_value))
	{
		RHBOPT_throw_StExcep(ev,BAD_OPERATION,BadFlag,NO);
	}

	if (ev->_major==NO_EXCEPTION)
	{
		RHBOPT_ASSERT(desc_value);

		data.arg_list=ORB_create_list_from_opdesc_value(
				SOMD_ORBObject,ev,
				desc_value,
				mods,
				CLIENT_SIDE);

		if (!ev->_major)
		{
			any a=RHBCDR_get_named_element(ev,desc_value,"mode");

			if (!ev->_major)
			{
				unsigned long mode=RHBCDR_cast_any_to_enum(ev,&a);

				{
					any a=RHBCDR_get_named_element(ev,desc_value,"result");

					if (!ev->_major)
					{
						TypeCode tc=RHBCDR_cast_any_to_TypeCode(ev,&a);

						result->argument._type=tcCopy(tc,ev);

						if (ORB_get_somModifier(NULL,ev,mods,
								"object_owns_result"))
						{
							result->arg_modes|=OBJ_OWNED;
						}

						status=0;
					}
				}

				switch (mode)
				{
				case OperationDef_NORMAL:
					if (result->argument._type)
					{
						result->arg_modes|=RESULT_OUT;
					}
					break;
				case OperationDef_ONEWAY:
					result->arg_modes|=INV_NO_RESPONSE;
					if (result->argument._type)
					{
						if (tk_void != TypeCode_kind(result->argument._type,ev))
						{
							RHBOPT_throw_StExcep(ev,BAD_OPERATION,BadFlag,NO);
						}
					}
					break;
				default:
					RHBOPT_throw_StExcep(ev,BAD_OPERATION,BadFlag,NO);
					break;
				}
			}
		}

		if (ev->_major==NO_EXCEPTION)
		{
			any a=RHBCDR_get_named_element(ev,desc_value,"contexts");

			if (ev->_major==NO_EXCEPTION)
			{
				switch (TypeCode_kind(a._type,ev))
				{
				case tk_sequence:
					if (a._value)
					{
						_IDL_SEQUENCE_string *seq=a._value;

						if (seq->_length)
						{
							result->arg_modes|=CTX_IN_MSG;
						}
					}
				}
			}
		}

		if (arg_list && !ev->_major)
		{
			*arg_list=data.arg_list;
			data.arg_list=NULL;
		}
	}

	RHBOPT_cleanup_pop();

    return status;
}

/* with a migrated method, can pick up the wrong
	implementation of this... */

SOM_Scope ORBStatus SOMLINK somdcprx_create_request(
	SOMDClientProxy SOMSTAR somSelf,
	Environment *ev,
	Context SOMSTAR ctx,
	char *op,
	NVList SOMSTAR args,
	NamedValue *nv,
	Request SOMSTAR *req,
	Flags flags)
{
	return SOMDClientProxy_parent_SOMDObject_create_request(
		somSelf,
		ev,
		ctx,
		op,
		args,
		nv,
		req,
		flags);
}
