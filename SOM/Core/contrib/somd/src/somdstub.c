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

#define SOMDServerStub_Class_Source

#ifdef _DEBUG
#	define DynamicTest_Class_Source
#endif

#include <rhbopt.h>
#include <rhbsomd.h>

#include <somdstub.ih>

SOM_Scope RepositoryId SOMLINK dynimpl_primary_interface(
SOMDServerStub SOMSTAR somSelf,
		Environment *ev,
/* in */ PortableServer_ObjectId *object_key,
/* in */ PortableServer_POA SOMSTAR poa)
{
/*SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf); 

	if (somThis->somdServer)
	{
		SOMClass SOMSTAR cls=SOMObject_somGetClass(somThis->somdServer);
	}
*/

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);

	return NULL;
}

struct ServerRequest_ExecuteParams_data
{
	NVList SOMSTAR args;
	Context SOMSTAR ctx;
	any result;
	_IDL_SEQUENCE_somModifier *modifiers;
	RHBContained *op;
};

RHBOPT_cleanup_begin(ServerRequest_ExecuteParams_cleanup,pv)

	struct ServerRequest_ExecuteParams_data *local=pv;
	Environment ev;

	SOM_InitEnvironment(&ev);

	if (local->result._value)
	{
		if (local->op && local->op->handler)
		{
			local->op->handler(local->op,&ev,&(local->result));
		}
		else
		{
			if (!ORB_get_somModifier(NULL,&ev,local->modifiers,"dual_owned_result"))
			{
				if (!ORB_get_somModifier(NULL,&ev,local->modifiers,"object_owns_result"))
				{
					SOMD_FreeType(
						&ev,
						local->result._value,
						local->result._type);
				}
			}
		}

		SOMFree(local->result._value);
	}

	if (local->args) 
	{
		long count=0;
		if (!NVList_get_count(local->args,&ev,&count))
		{
			long l=0;
			while (l < count)
			{
				NamedValue nv=RHBSOMD_Init_NamedValue;
				NVList_get_item(local->args,&ev,
						l++,
						&nv.name,
						&nv.argument._type,
						&nv.argument._value,
						&nv.len,
						&nv.arg_modes);

				SOMD_FreeType(&ev,nv.argument._value,nv.argument._type);

				SOMFree(nv.argument._value);
			}
		}

		somReleaseObjectReference(local->args);
	}

	if (local->ctx) 
	{
		Context_somFree(local->ctx);
		local->ctx=NULL;
	}

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

static void ServerRequest_ExecuteParams(
					ServerRequest SOMSTAR somSelf,
					Environment *ev,
					SOMDObject SOMSTAR server,
					SOMObject SOMSTAR target,
					somId id,
					any *desc_value,
					_IDL_SEQUENCE_somModifier *modifiers,
					RHBContained *op)
{
	unsigned int i=0;
	any desc_parameter={NULL,NULL};
	struct ServerRequest_ExecuteParams_data locals;
/*	RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);*/

	locals.ctx=NULL;
	locals.result._type=TC_void;
	locals.result._value=NULL;
	locals.args=NULL;
	locals.modifiers=modifiers;
	locals.op=op;

	RHBOPT_cleanup_push(ServerRequest_ExecuteParams_cleanup,&locals);

	desc_parameter=RHBCDR_get_named_element(ev,desc_value,"parameter");

	if (!desc_parameter._type)
	{
		desc_parameter=RHBCDR_get_named_element(ev,desc_value,"parameters");
	}

	i=RHBCDR_count_elements(ev,&desc_parameter);

	if (i)
	{
		if (!ORB_create_list(SOMD_ORBObject,ev,i,&locals.args))
		{
			unsigned long j=0;
			while (j < i)
			{
				long mode=0;
				NamedValue nv=RHBSOMD_Init_NamedValue;
				any nth=RHBCDR_get_nth_element(ev,&desc_parameter,j);
				any el=RHBCDR_get_named_element(ev,&nth,"name");

				nv.name=RHBCDR_cast_any_to_string(ev,&el);

				el=RHBCDR_get_named_element(ev,&nth,"mode");

				mode=RHBCDR_cast_any_to_long(ev,&el);

				switch (mode)
				{
				case ParameterDef_IN:
					nv.arg_modes=ARG_IN;
					break;
				case ParameterDef_OUT:
					nv.arg_modes=ARG_OUT;
					break;
				case ParameterDef_INOUT:
					nv.arg_modes=ARG_INOUT;
					break;
				}

				el=RHBCDR_get_named_element(ev,&nth,"type");

				nv.argument._type=RHBCDR_cast_any_to_TypeCode(ev,&el);
				
				NVList_set_item(locals.args,ev,j,
						nv.name,
						nv.argument._type,
						nv.argument._value,
						nv.len,
						nv.arg_modes);

				j++;
			}
		}
	}

	ServerRequest_arguments(somSelf,ev,&locals.args);

	if (!ev->_major)
	{
		any a=RHBCDR_get_named_element(ev,desc_value,"contexts");
		size_t return_len=0;

		unsigned long cc=RHBCDR_count_elements(ev,&a);

		if (cc)
		{
			locals.ctx=ServerRequest_ctx(somSelf,ev);
		}

		/* now we have decoded the parameters, so set them up now */
		a=RHBCDR_get_named_element(ev,desc_value,"result");
		locals.result._type=RHBCDR_cast_any_to_TypeCode(ev,&a);

		switch (TypeCode_kind(locals.result._type,ev))
		{
		case tk_array:
			/* actual object should allocate the array */
			break;
		default:
			return_len=TypeCode_size(locals.result._type,ev);

			if (return_len < sizeof(void *))
			{
				return_len=sizeof(void *);
			}
		}

		if (!ev->_major)
		{
			if (return_len)
			{
				locals.result._value=SOMMalloc(return_len);

				if (locals.result._value)
				{	
					memset(locals.result._value,0,return_len);
				}
				else
				{
					RHBOPT_throw_StExcep(ev,NO_MEMORY,DispatchError,NO);
				}
			}

			if (!ev->_major)
			{
				SOMDServer_somdDispatchArgs(
					server,ev,locals.ctx,
					target,id,
					&locals.result,
					locals.args);
			}

			ServerRequest_set_result(somSelf,ev,&locals.result);
		}
	}

	
	RHBOPT_cleanup_pop();
}

struct ServerRequest_ExecuteMeta
{
	somId clsId;
	SOMClassMgr SOMSTAR mgr;
	SOMClass SOMSTAR clsObj;
	char *repId;
	char *className;
	char *absId;
	TypeCode tc;
	SOMObject SOMSTAR obj;
	SOMObject SOMSTAR ref;
	NVList SOMSTAR args;
};

RHBOPT_cleanup_begin(ServerRequest_ExecuteMeta_cleanup,pv)

	struct ServerRequest_ExecuteMeta *data=pv;

	if (data->className) SOMFree(data->className);
	if (data->repId) SOMFree(data->repId);
	if (data->clsId) SOMFree(data->clsId);
	if (data->absId) SOMFree(data->absId);
	if (data->mgr) { somReleaseClassManagerReference(data->mgr); }
	if (data->clsObj) { somReleaseClassReference(data->clsObj); }
	if (data->tc)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		TypeCode_free(data->tc,&ev);
		SOM_UninitEnvironment(&ev);
	}

	if (data->obj) 
	{
		somReleaseObjectReference(data->obj);
	}

	if (data->ref) 
	{
		somReleaseObjectReference(data->ref);
	}

	if (data->args)
	{
		somReleaseObjectReference(data->args);
	}

RHBOPT_cleanup_end

static boolean ServerRequest_ExecuteMeta(
			SOMDServerStubData *somThis,
			ServerRequest SOMSTAR somSelf,
			Environment *ev,
			SOMObject SOMSTAR target)
{
	struct ServerRequest_ExecuteMeta data={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	boolean RHBOPT_volatile execMeta=0;
	char *operation=ServerRequest__get_operation(somSelf,ev);
	RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

	RHBOPT_cleanup_push(ServerRequest_ExecuteMeta_cleanup,&data);

	if (!strcmp(operation,"_is_a"))
	{
/*		NamedValue n=RHBSOMD_Init_NamedValue;*/
		any result={NULL,NULL};
		boolean ret=0;

		data.args=NVListNew();

		result._type=TC_boolean;
		result._value=&ret;

		NVList_add_item(data.args,ev,
				"repId",
				TC_string,
				&data.repId,
				0,
				ARG_IN);

		ServerRequest_arguments(somSelf,ev,&data.args);

		if (!ev->_major)
		{
			data.className=RHBORB_convert_interface_to_class(orb,data.repId);
		}

		if (data.className)
		{
			data.clsId=somIdFromString(data.className);

			if (data.clsId)
			{
				data.mgr=somGetClassManagerReference();

				if (data.mgr)
				{
					data.clsObj=SOMClassMgr_somFindClass(data.mgr,data.clsId,0,0);

					if (data.clsObj)
					{
						ret=SOMObject_somIsA(target,data.clsObj);
					}
					else
					{
						debug_somPrintf(("%s::_is_a(%s,%s) class does not exist\n",
								target->mtab->className,
								data.repId,
								data.className));
						ret=0;
					}
				}
			}
		}

		ServerRequest_set_result(somSelf,ev,&result);

		execMeta=1;
	}
	else
	{
		/* _interface */

		if (!strcmp(operation,"_interface"))
		{
			any result={NULL,NULL};
			char *c=target->mtab->className;
			size_t len=strlen(c);

			/**************************************
			 *
			 * here we call SOMDServer_somdRefFromSOMObj
			 * to get a public interface to the object
			 * then use the type_id to determine the interface
			 * 
			 * currently it just uses ORB_lookup_id when 
			 * it should really acquire a CORBA_InterfaceDef
			 * for this type_id as the type of object 
			 * returned should be ::CORBA::InterfaceDef not
			 * SOM's ::InterfaceDef
			 *
			 */

			ServerRequest_arguments(somSelf,ev,&data.args);

			result._type=data.tc=somdTC_CORBA_InterfaceDef;
			result._value=&data.obj;

			if (somThis->somdServer)
			{
				data.ref=SOMDServer_somdRefFromSOMObj(
					somThis->somdServer,ev,target);

				if (data.ref && !ev->_major)
				{
					data.repId=SOMDObject__get_type_id(data.ref,ev);

					if (data.repId && !ev->_major)
					{
						char *id=data.repId;

						if (id[0]!=':')
						{
							data.absId=RHBORB_id_to_abs(orb,id);

							if (data.absId) 
							{
								id=data.absId;
							}
						}

						data.obj=ORB_lookup_id(SOMD_ORBObject,ev,NULL,id);
					}
				}
			}
			
			if ((!data.obj) && (!data.repId) && !ev->_major)
			{
				data.repId=SOMMalloc(3+len);
				data.repId[0]=':';
				data.repId[1]=':';
				memcpy(data.repId+2,c,len+1);

				data.obj=ORB_lookup_id(SOMD_ORBObject,ev,NULL,data.repId);
			}

			ServerRequest_set_result(somSelf,ev,&result);

			execMeta=1;
		}
		else
		{
		/* _implementation */
#if 0
			if (!strcmp(operation,"_implementation"))
			{
				any result={NULL,NULL};

				/* return reference to ImplementationDef for this server */

				ServerRequest_arguments(somSelf,ev,&data.args);

				data.obj=RHBImplementationDef_get_wrapper(somThis->impl,ev,0);

				result._type=data.tc=TypeCodeNew(tk_objref,"::ImplementationDef");
				result._value=&data.obj;

				RHBServerRequest_set_result(somThis,ev,&result);

				execMeta=1;
			}
			else
#endif
			{
				any result={NULL,NULL};

				/* _non_existent */

				debug_somPrintf(("unknown operation %s::%s\n",
						SOMObject_somGetClassName(target),
						somThis->header.operation));

				RHBOPT_throw_StExcep(ev,BAD_OPERATION,BadMethodName,YES);

				ServerRequest_set_result(somSelf,ev,&result);
			}
		}
	}

	RHBOPT_cleanup_pop();

	return execMeta;
}

struct dynimpl_invoke
{
	somId id;
	somId methodDescriptor;
	char *op_id;
	RHBContained *op;
	ServerRequest SOMSTAR request;
};

RHBOPT_cleanup_begin(dynimpl_invoke_cleanup,pv)

struct dynimpl_invoke *data=pv;

	if (data->id) SOMFree(data->id);
#ifdef USE_APPLE_SOM
	if (data->methodDescriptor) SOMFree(data->methodDescriptor);
#endif
	if (data->op_id) SOMFree(data->op_id);
	if (data->op) RHBContained_Release(data->op);

RHBOPT_cleanup_end

SOM_Scope void SOMLINK dynimpl_invoke(
	SOMDServerStub SOMSTAR somSelf,
	Environment *ev,
	/* in */ ServerRequest SOMSTAR request)
{
	if (ev && !ev->_major)
	{
		SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf);
		struct dynimpl_invoke data={NULL,NULL,NULL,NULL,0};
		RHBORB *orb;
		char *op;
#ifdef USE_APPLE_SOM
		somKernelId kid;
#endif

		RHBOPT_cleanup_push(dynimpl_invoke_cleanup,&data);

		data.request=request;

		orb=ORB__get_c_orb(SOMD_ORBObject,ev);

		op=ServerRequest__get_operation(request,ev);

		data.id=somIdFromString(op);

	#ifdef USE_APPLE_SOM
		kid=SOMClass_somGetMethodDescriptor(
					somThis->somdServant->mtab->classObject,data.id);
		if (kid)
		{
			data.methodDescriptor=somConvertAndFreeKernelId(kid);
		}
	#else
		data.methodDescriptor=SOMClass_somGetMethodDescriptor(
				somThis->somdServant->mtab->classObject,data.id);
	#endif

		if (data.methodDescriptor)
		{
			data.op_id=somMakeStringFromId(data.methodDescriptor);

			data.op_id=RHBORB_leading_colons(orb,data.op_id);

			data.op=RHBORB_get_contained(orb,ev,data.op_id);

			if (data.op)
			{
				ServerRequest_ExecuteParams(request,
					ev,
					somThis->somdServer,
					somThis->somdServant,
					data.id,
					&data.op->desc_value,
					&data.op->modifiers,
					data.op);
			}
			else
			{
				RHBOPT_throw_StExcep(ev,BAD_OPERATION,DispatchError,NO);
			}
		}
		else
		{
			ServerRequest_ExecuteMeta(
					somThis,
					request,
					ev,
					somThis->somdServant);
		}

		RHBOPT_cleanup_pop();
	}
}

#ifdef SOMDServerStub_somDefaultInit
SOM_Scope void SOMLINK dynimpl_somDefaultInit(
	SOMDServerStub SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SOMDServerStubData *somThis;

	SOMDServerStub_BeginInitializer_somDefaultInit
	SOMDServerStub_Init_PortableServer_DynamicImpl_somDefaultInit(somSelf,ctrl);
	RHBOPT_unused(somThis);
}
#else
SOM_Scope void SOMLINK dynimpl_somInit(
	SOMDServerStub SOMSTAR somSelf)
{
/* SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf);*/

	SOMDServerStub_parent_PortableServer_DynamicImpl_somInit(somSelf);
	RHBOPT_unused(somThis);
}
#endif

#ifdef SOMDServerStub_somDestruct
SOM_Scope void SOMLINK dynimpl_somDestruct(
	SOMDServerStub SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SOMDServerStubData *somThis;

	SOMDServerStub_BeginDestructor
	RHBOPT_unused(somThis);
	SOMDServerStub_EndDestructor
}
#else
SOM_Scope void SOMLINK dynimpl_somUninit(
	SOMDServerStub SOMSTAR somSelf)
{
/* SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf);*/

	SOMDServerStub_parent_PortableServer_DynamicImpl_somUninit(somSelf);
}
#endif

SOM_Scope void SOMLINK dynimpl__set_somdServer(
	SOMDServerStub SOMSTAR somSelf,
	Environment *ev,
	SOMDServer SOMSTAR obj)
{
SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf);

	somThis->somdServer=obj;
}

SOM_Scope SOMDServer SOMSTAR SOMLINK dynimpl__get_somdServer(
	SOMDServerStub SOMSTAR somSelf,
	Environment *ev)
{
/* SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf); */

	return NULL;
}

SOM_Scope void SOMLINK dynimpl__set_somdServant(
	SOMDServerStub SOMSTAR somSelf,
	Environment *ev,
	SOMObject SOMSTAR obj)
{
SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf);

	somThis->somdServant=obj;
}

SOM_Scope SOMObject SOMSTAR SOMLINK dynimpl__get_somdServant(
	SOMDServerStub SOMSTAR somSelf,
	Environment *ev)
{
/* SOMDServerStubData *somThis=SOMDServerStubGetData(somSelf);*/

	return NULL;
}

