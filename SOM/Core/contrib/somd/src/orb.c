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
#include <somuutil.h>
#include <dynany.h>

#ifndef SOM_Module_orb_Source
#define SOM_Module_orb_Source
#endif
#define ORB_Class_Source
#define M_ORB_Class_Source

#include "orb.ih"

/* these two get defined by dynany.idl */
#ifdef _type
	#undef _type
#endif
#ifdef _value
	#undef _value
#endif


SOM_Scope RHBORB * SOMLINK orb__get_c_orb(ORB SOMSTAR somSelf,Environment *ev)
{
	if (somSelf)
	{
		ORBData *somThis=ORBGetData(somSelf);

		if (somThis->c_orb)
		{
			return somThis->c_orb;
		}

		if (somThis->failure_ev._major!=NO_EXCEPTION)
		{
			StExcep_UNKNOWN *params=NULL;

			if (somThis->failure_ev._major==SYSTEM_EXCEPTION)
			{
				StExcep_UNKNOWN *ex=somThis->failure_ev.exception._params;

				if (ex)
				{
					params=SOMMalloc(sizeof(*params));

					if (params)
					{
						params->minor=ex->minor;
						params->completed=ex->completed;
					}
				}
			}

			somSetException(ev,
				somThis->failure_ev._major,
				somExceptionId(&somThis->failure_ev),
				params);
		}
	}

	RHBOPT_throw_StExcep(ev,NO_RESOURCES,NoSOMDInit,NO);

	return NULL;
}

SOM_Scope corbastring  SOMLINK orb_object_to_string(ORB SOMSTAR somSelf, 
                                                Environment *ev, 
                                               SOMDObject SOMSTAR obj)
{
    ORBData *somThis = ORBGetData(somSelf);

	if (obj)
	{
		if (SOMObject_somIsA(obj,somThis->c_orb->somClass_SOMDObject_ref))
		{
			return RHBORB_object_to_string(
				somThis->c_orb,
				ev,obj);
		}
		else
		{
			RHBOPT_throw_StExcep(ev,MARSHAL,BadObjref,NO);
		}

	}

	return NULL;
}

SOM_Scope SOMDObject SOMSTAR  SOMLINK orb_string_to_object(ORB SOMSTAR somSelf, 
                                                     Environment *ev, 
                                                    corbastring str)
{
    ORBData *somThis = ORBGetData(somSelf); 

	if (somThis->c_orb)
	{
		return RHBORB_string_to_object(somThis->c_orb,ev,str);
	}

	return NULL;
}

SOM_Scope ORBStatus SOMLINK orb_create_list(ORB SOMSTAR somSelf,  Environment *ev, 
                                             long count, NVList SOMSTAR* new_list)
{
    ORBData *somThis = ORBGetData(somSelf);
	RHBORB *orb=somThis->c_orb;
	NVList SOMSTAR list=SOMClass_somNew(orb->somClass_NVList_ref);

	while (count--)
	{
		NVList_add_item(list,ev,NULL,NULL,NULL,0,0);
	}

	*new_list=list;

    return 0;
}

struct orb_create_operation_list
{
	any desc_value;
	_IDL_SEQUENCE_somModifier mods;
	NVList SOMSTAR list;
	unsigned short describe_flags;
};

RHBOPT_cleanup_begin(orb_create_operation_list_cleanup,pv)

	struct orb_create_operation_list *data=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);
	SOMD_FreeType(&ev,&data->desc_value,TC_any);
	SOMD_FreeType(&ev,&data->mods,somdTC_sequence_somModifier);
	SOMD_FreeType(&ev,&data->list,TC_Object);
	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

SOM_Scope ORBStatus SOMLINK orb_create_operation_list(
		ORB SOMSTAR somSelf, 
		Environment *ev, 
		OperationDef SOMSTAR oper, 
		NVList SOMSTAR * new_list)
{
	ORBStatus RHBOPT_volatile rc=SOMDERROR_UnexpectedNULL;

	*new_list=NULL;

	if (oper && !ev->_major)
	{
		struct orb_create_operation_list data={{NULL,NULL},{0,0,NULL},NULL,0};

		RHBOPT_cleanup_push(orb_create_operation_list_cleanup,&data);
		/* this should really get the modifiers in parallel...
		   and confirm the type of IR
		*/

		ORB_contained_describe(somSelf,ev,oper,
				&data.describe_flags,
				&data.desc_value,&data.mods,NULL,NULL);

		if (!ev->_major)
		{
			data.list=ORB_create_list_from_opdesc_value(
					somSelf,ev,&data.desc_value,&data.mods,0);

			if (!ev->_major)
			{
				*new_list=data.list;
				data.list=NULL;

				rc=0;
			}
		}

		RHBOPT_cleanup_pop();
	}
	else
	{
		RHBOPT_throw_StExcep(ev,BAD_OPERATION,UnexpectedNULL,NO);
	}

    return rc;
}

SOM_Scope NVList SOMSTAR SOMLINK orb_create_list_from_opdesc_value(ORB SOMSTAR somSelf,
			Environment *ev,
			any *value,
			_IDL_SEQUENCE_somModifier *mods,
			long flags)
{
	NVList SOMSTAR list=NULL;
	long count=0;
	int i=0;

	any parameters=RHBCDR_get_named_element(ev,value,"parameters");

	if (!parameters._type)
	{
		parameters=RHBCDR_get_named_element(ev,value,"parameter");

		if (!parameters._type)
		{
			RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,NO);

			return NULL;
		}
	}

	count=RHBCDR_count_elements(ev,&parameters);

	ORB_create_list(somSelf,ev,0,&list);

	while (i < count)
	{
		any param=RHBCDR_get_nth_element(ev,&parameters,i);
		any param_name=RHBCDR_get_named_element(ev,&param,"name");
		any param_type=RHBCDR_get_named_element(ev,&param,"type");
		any param_mode=RHBCDR_get_named_element(ev,&param,"mode");

		char *type_name=RHBCDR_cast_any_to_string(ev,&param_name);
		TypeCode type_type=RHBCDR_cast_any_to_TypeCode(ev,&param_type);
		unsigned long type_mode=RHBCDR_cast_any_to_enum(ev,&param_mode);

		switch (type_mode)
		{
		case ParameterDef_IN:
			type_mode=ARG_IN;
			break;
		case ParameterDef_OUT:
			type_mode=ARG_OUT;
			break;
		case ParameterDef_INOUT:
			type_mode=ARG_INOUT;
			break;
		}

		type_mode|=(flags & (SERVER_SIDE|CLIENT_SIDE));

		NVList_add_item(list,ev,
			type_name,
			type_type,
			NULL,
			0,
			type_mode);

		i++;
	}

	return list;
}

SOM_Scope ORBStatus  SOMLINK orb_get_default_context(
		ORB SOMSTAR somSelf, 
		Environment *ev, 
		Context SOMSTAR * ctx)
{
	if (ev->_major)
	{
		*ctx=NULL;

		return SOMDERROR_BadEnvironment;
	}
	else
	{
		ORBData *somThis=ORBGetData(somSelf);
		Context SOMSTAR c=SOMClass_somNew(somThis->c_orb->somClass_Context_ref);

		*ctx=c;

		if (!c)
		{
			RHBOPT_throw_StExcep(ev,BAD_CONTEXT,UnexpectedNULL,YES)

			return SOMDERROR_UnexpectedNULL;
		}
	}

    return 0;
}

#ifdef ORB_somDefaultInit
SOM_Scope void SOMLINK orb_somDefaultInit(
	ORB SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	SOMD_Globals *globals=&somd_globals;
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	ORBData *somThis;

	ORB_BeginInitializer_somDefaultInit
	ORB_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
#else
SOM_Scope void  SOMLINK orb_somInit(ORB SOMSTAR somSelf)
{
	Environment ev;
    ORBData *somThis=ORBGetData(somSelf);
	SOMD_Globals *globals=&somd_globals;

	SOM_InitEnvironment(&ev);
#	ifdef ORB_parent_SOMRefObject_somInit
    ORB_parent_SOMRefObject_somInit(somSelf);
#	else
    ORB_parent_SOMObject_somInit(somSelf);
#	endif
#endif

	if (!SOMD_ORBObject)
	{
		SOMD_ORBObject=somSelf;
	}

	somThis->c_orb=RHBORBNew(globals,&somThis->failure_ev);

	if (somThis->c_orb)
	{
		globals->lpVtbl->Init(globals,&somThis->failure_ev);
	}
}

#ifdef ORB_somDestruct
SOM_Scope void SOMLINK orb_somDestruct(
	ORB SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	SOMD_Globals *globals=NULL;
	SOMObject SOMSTAR obj;
	Environment ev;
	RHBORB *orb;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	ORBData *somThis;

	ORB_BeginDestructor
#else
SOM_Scope void  SOMLINK orb_somUninit(ORB SOMSTAR somSelf)
{
	RHBORB *orb;
    ORBData *somThis = ORBGetData(somSelf);
	Environment ev;
	SOMObject SOMSTAR obj;
	SOMD_Globals *globals=NULL;
#endif

	SOM_InitEnvironment(&ev);

	if (SOMMSingleInstanceClassData.sommSingleInstanceFreed)
	{
		SOMMSingleInstance_sommSingleInstanceFreed(somSelf->mtab->classObject,&ev,somSelf);
	}

	RHBORB_guard(somThis->c_orb)

	if (SOMD_ORBObject==somSelf)
	{
		SOMD_ORBObject=NULL;
	}

	RHBORB_unguard(somThis->c_orb)

	obj=SOMD_ObjectMgr;
	SOMD_ObjectMgr=NULL;

	if (obj)
	{
		somReleaseObjectReference(obj);
	}

	obj=SOMD_ImplRepObject;
	SOMD_ImplRepObject=NULL;

	if (obj)
	{
		somReleaseObjectReference(obj);
	}

#ifdef USE_SELECT
	obj=SOMD_SocketObject;
	SOMD_SocketObject=NULL;

	if (obj) 
	{
		somReleaseObjectReference(obj);
	}
#endif

	RHBORB_guard(somThis->c_orb)

	orb=somThis->c_orb;

	somThis->c_orb=NULL;

	RHBORB_unguard(orb)

	if (orb)
	{
		Environment ev;
		globals=orb->globals;
		SOM_InitEnvironment(&ev);
		RHBSOM_Trace("Calling RHBORB_Close")
		RHBORB_Close(orb,&ev);
		RHBSOM_Trace("Calling RHBORB_Release")
		RHBORB_Release(orb);
		RHBSOM_Trace("Calling SOM_UninitEnvironment")
		SOM_UninitEnvironment(&ev);
	}
	
	somExceptionFree(&somThis->failure_ev);

#ifdef ORB_somDestruct
	ORB_EndDestructor
#else
#	ifdef ORB_parent_SOMRefObject_somUninit
    ORB_parent_SOMRefObject_somUninit(somSelf);
#	else
    ORB_parent_SOMObject_somUninit(somSelf);
#	endif
#endif
	if (globals) 
	{
		globals->lpVtbl->Uninit(globals,&ev);
	}
	SOM_UninitEnvironment(&ev);
}

static char szInterfaceRepository[]=ORB_initial_InterfaceRepository;
static char szNameService[]=ORB_initial_NameService;
static char szCodecFactory[]=ORB_initial_CodecFactory;
static char szDynAnyFactory[]=ORB_initial_DynAnyFactory;
static char *ORB_initial_services[]=
{
	szInterfaceRepository,szNameService,szCodecFactory,szDynAnyFactory
};

static ORB_ObjectIdList seqObjectIdList={
	sizeof(ORB_initial_services)/sizeof(ORB_initial_services[0]),
	sizeof(ORB_initial_services)/sizeof(ORB_initial_services[0]),
	ORB_initial_services};

SOM_Scope ORB_ObjectIdList SOMLINK orb_list_initial_services(
	ORB SOMSTAR somSelf,
	Environment *ev)
{
	ORB_ObjectIdList seq={0,0,NULL};

	RHBOPT_unused(somSelf)

	if (ev->_major) return seq;

	seq._length=seqObjectIdList._length;
	seq._maximum=seqObjectIdList._maximum;

	if (seqObjectIdList._length)
	{
		unsigned int i=0;

		seq._buffer=SOMMalloc(sizeof(seq._buffer[0])*seq._length);

		while (i < seq._length)
		{
			seq._buffer[i]=somd_dupl_string(seqObjectIdList._buffer[i]);
			i++;
		}
	}

	return seq;
}

struct orb_rir
{
	char *str;
};

RHBOPT_cleanup_begin(orb_rir_cleanup,pv)

struct orb_rir *data=pv;

	if (data->str) SOMFree(data->str);

RHBOPT_cleanup_end

SOM_Scope SOMObject SOMSTAR SOMLINK orb_resolve_initial_references(
	ORB SOMSTAR somSelf,
	Environment *ev,
	/* in */ ORB_ObjectId identifier)
{
	ORBData *somThis=ORBGetData(somSelf);
	SOMObject SOMSTAR RHBOPT_volatile retVal=NULL;

	if (ev->_major==NO_EXCEPTION)
	{
		struct orb_rir data={NULL};

		RHBOPT_cleanup_push(orb_rir_cleanup,&data);

		if (0==strcmp(identifier,szInterfaceRepository))
		{
			retVal=RHBORB_acquire_interface_repository(somThis->c_orb,ev,szInterfaceRepository);
		}
		else
		{
			if (0==strcmp(identifier,szNameService))
			{
				char *p=somutgetenv("SOMNMOBJREF","[somnm]");

				if (p)
				{
					data.str=somd_dupl_string(p);

					retVal=ORB_string_to_object(somSelf,ev,data.str);
				}

				if (!retVal)
				{
					RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,YES);
				}
			}
			else
			{
				if (0==strcmp(identifier,szCodecFactory))
				{
					retVal=IOP_CodecFactory_somDuplicateReference(somThis->c_orb->iop_codecFactory);
				}
				else
				{
					if (0==strcmp(identifier,szDynAnyFactory))
					{
						retVal=
				#ifdef somNewObject
						somNewObject(DynamicAny_DynAnyFactory);
				#else
						DynamicAny_DynAnyFactoryNew();
				#endif
					}
					else
					{
						RHBOPT_throw_StExcep(ev,BAD_PARAM,BadObjref,YES);
					}
				}
			}
		}

		RHBOPT_cleanup_pop();
	}

	return retVal;
}

SOM_Scope boolean SOMLINK orb_work_pending(
	ORB SOMSTAR somSelf,Environment *ev)
{
	/* this should check the incoming request queue really */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope void SOMLINK orb_perform_work(
	ORB SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)

	if (SOMD_SOMOAObject)
	{
		SOMOA_execute_next_request(
			SOMD_SOMOAObject,
			ev,
			SOMD_NO_WAIT);
	}
}

SOM_Scope void SOMLINK orb_shutdown(
	ORB SOMSTAR somSelf,
	Environment *ev,
	boolean wait_for_completion)
{
	ORBData *somThis=ORBGetData(somSelf);
	RHBORB *orb;

	RHBOPT_unused(wait_for_completion)

	RHBORB_guard(0)

	orb=somThis->c_orb;

	if (orb)
	{
		RHBORB_AddRef(orb);
	}

	RHBORB_unguard(0)

	if (orb)
	{
		RHBORB_shutdown(orb,ev);
		RHBORB_Release(orb);
	}
}

SOM_Scope void SOMLINK orb_run(
	ORB SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)

	if (SOMD_SOMOAObject)
	{
		SOMOA_execute_request_loop(
			SOMD_SOMOAObject,
			ev,
			SOMD_WAIT);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,YES);
	}
}


SOM_Scope void SOMLINK orb_somDumpSelfInt(ORB SOMSTAR somSelf,long level)
{
ORBData *somThis=ORBGetData(somSelf);
RHBORB *orb;

	orb=somThis->c_orb;

	RHBORB_guard(orb)

	somPrefixLevel(level); dump_somPrintf(("c_orb=%p\n",orb));

	if (orb)
	{
		if (orb->iface)
		{
			RHBInterfaceDef *i=orb->iface;

			while (i)
			{
				somPrefixLevel(level); dump_somPrintf(("iface=%s\n",i->id));
				i=i->next;
			}
		}

		if (orb->impls)
		{
			RHBImplementationDef *i=orb->impls;

			while (i)
			{
				somPrefixLevel(level);
				dump_somPrintf(("impl=%s:%d\n",
						i->address.host,
						(int)i->address.port));

				i=i->next;
			}
		}
	}

	RHBORB_unguard(orb)
}

SOM_Scope boolean SOMLINK orb__get_stringToObject30(
		ORB SOMSTAR somSelf,
		Environment *ev)
{
	return 1;
}

SOM_Scope void SOMLINK orb__set_stringToObject30(
		ORB SOMSTAR somSelf,
		Environment *ev,
		boolean stringToObject30)
{
	RHBOPT_throw_StExcep(ev,NO_PERMISSION,NoAccess,NO);
}

#ifdef USE_SELECT
static void SOMLINK async_proc(SOMObject SOMSTAR event,void *token)
{
	RHBORB *somThis=token;
	if (event)
	{
		Environment ev;
		struct RHBAsyncEvent *e;
		SOM_InitEnvironment(&ev);
/*		somPrintf("async_proc\n");
		SOMObject_somDumpSelf(event,0);*/
		e=SOMEClientEvent_somevGetEventClientData(event,&ev);
		if (e)
		{
			e->callback(e);
		}
		SOM_UninitEnvironment(&ev);
	}
	if (somThis)
	{
		if (somThis->events.user_callback)
		{
			somThis->events.user_callback(event,token);
		}
	}
}

#if defined(_WIN32) && defined(_DEBUG) && 0
static void test_async_event(struct RHBAsyncEvent *e)
{
	SOMObject_somDumpSelf(e->refcon,0);
	RHBAsyncEvent_uninit(e);
}
#endif


#endif

SOM_Scope void SOMLINK orb_register_callback(
		ORB SOMSTAR somSelf,
		Environment *ev,
		SOMObject SOMSTAR emanObj,
		somMethodPtr func)
{
#ifdef USE_THREADS
	RHBOPT_unused(emanObj)
	RHBOPT_unused(func)
#else
	ORBData *somThis=ORBGetData(somSelf);
	RHBORB *c_orb=somThis->c_orb;
	if (c_orb->events.user_eman != emanObj)
	{
		if (c_orb->events.user_eman)
		{
			SOMEEMan SOMSTAR oldman=c_orb->events.user_eman;
			/* need to clear up any references to this eman */

			if (c_orb)
			{
				RHBSocketEvent *se=c_orb->events.eventList;
				while (se)
				{
					long read_Token=se->read_token;
					long write_Token=se->write_token;
					long except_Token=se->except_token;
					SOMEEMRegisterData SOMSTAR rd=se->regData;

					se->read_token=0;
					se->write_token=0;
					se->except_token=0;

					se->regData=NULL;

					if (read_Token)	SOMEEMan_someUnRegister(oldman,ev,read_Token);
					if (write_Token) SOMEEMan_someUnRegister(oldman,ev,write_Token);
					if (except_Token) SOMEEMan_someUnRegister(oldman,ev,except_Token);

					if (rd)	somReleaseObjectReference(rd);

					if (se) se=se->next;
				}
			}

			SOMEEMan_someUnRegister(oldman,ev,c_orb->events.async_proc_token);

			c_orb->events.async_proc_token=0;
		}

		if (emanObj)
		{
			SOMEEMRegisterData SOMSTAR r=SOMEEMRegisterDataNew();

			SOMEEMRegisterData_someClearRegData(r,ev);
			somva_SOMEEMRegisterData_someSetRegDataEventMask(r,ev,EMClientEvent,NULL);
			SOMEEMRegisterData_someSetRegDataClientType(r,ev,c_orb->events.async_proc_type);

			c_orb->events.async_proc_token=SOMEEMan_someRegisterProc(emanObj,ev,
					r,
					async_proc,
					c_orb);

			SOMEEMRegisterData_somRelease(r);
		}

		c_orb->events.user_eman=emanObj;
		c_orb->events.user_callback=(SOMD_Callback *)func;

#if defined(_WIN32) && defined(_DEBUG) && 0
		if (emanObj)
		{
			static struct RHBAsyncEvent e2;

			e2.callback=test_async_event;
			e2.refcon=RHBAsyncEvent_init(&e2,ev,c_orb);
			RHBAsyncEvent_post(&e2);
		}
#endif
	}
#endif
}

SOM_Scope char * SOMLINK orb_get_somModifier(
		ORB SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_somModifier *modifiers,char *name)
{
	if (modifiers && name && !ev->_major)
	{
		int i=modifiers->_length;
		if (i)
		{
			somModifier *n=modifiers->_buffer;
			while (i--)
			{
				if (0==strcmp(n->name,name))
				{
					if (n->value) return n->value;

					return "";
				}

				n++;
			}
		}
	}

	return NULL;
}

struct orb_describe_contained
{
	unsigned short *flags;
	Request SOMSTAR req_describe;
	Request SOMSTAR req_mods;
	Request SOMSTAR req_abs;
	Request SOMSTAR req_id;
	Contained_Description desc_CORBA_1_1;
	CORBA_Contained_Description desc_CORBA_2_0;
	struct
	{
		Environment *ev;
		unsigned short *flags;
		any *desc_value;
		_IDL_SEQUENCE_somModifier *mods;
	} args;
};

RHBOPT_cleanup_begin(orb_describe_contained_cleanup,pv)

	struct orb_describe_contained *data=pv;
	Environment ev;

	SOM_InitEnvironment(&ev);

	if (data->flags[0]==ORB_DESCRIBE_FREE)
	{
		SOMD_FreeType(&ev,&data->desc_CORBA_1_1.name,TC_string);
	}

	data->desc_CORBA_2_0.kind=0;

	SOMD_FreeType(&ev,&data->desc_CORBA_1_1.value,TC_any);
	SOMD_FreeType(&ev,&data->desc_CORBA_2_0.value,TC_any);

	if (data->req_id)	somReleaseObjectReference(data->req_id);
	if (data->req_describe)	somReleaseObjectReference(data->req_describe);
	if (data->req_mods)	somReleaseObjectReference(data->req_mods);
	if (data->req_abs) somReleaseObjectReference(data->req_abs);

	if (data->args.ev->_major!=NO_EXCEPTION)
	{
		switch (data->args.flags[0])
		{
		case ORB_DESCRIBE_FREE:
			if (data->args.mods)
			{
				SOMD_FreeType(&ev,data->args.mods,somdTC_sequence_somModifier);
			}
			break;
		case ORB_DESCRIBE_SOMIR:
			break;
		}
	}

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

#ifdef USE_APPLE_SOM
static char sz__get_somModifiers[]="_get_somModifiers";
static char sz_describe[]="describe";
static char sz__get_absolute_name[]="_get_absolute_name";
static char sz__get_id[]="_get_id";
#endif

SOM_Scope void SOMLINK orb_contained_describe(
	ORB SOMSTAR somSelf,
	Environment *ev,
	SOMObject SOMSTAR contained,
	unsigned short *flags,
	any *desc_value,
	_IDL_SEQUENCE_somModifier *mods,
	char **absolute_name,
	char **id)
{
struct orb_describe_contained data={NULL,NULL,NULL,NULL,
	NULL,
	{NULL,{NULL,NULL}},
	{0,{NULL,NULL}},
	{NULL,NULL,NULL,NULL}};

	RHBOPT_ASSERT(flags);
	RHBOPT_ASSERT(mods ? !mods->_length : 1);
	RHBOPT_ASSERT(mods ? !mods->_maximum : 1);
	RHBOPT_ASSERT(mods ? !mods->_length : 1);

	data.flags=flags;
	data.args.ev=ev;
	data.args.flags=flags;
	data.args.mods=mods;
	data.args.desc_value=desc_value;

	*flags=ORB_DESCRIBE_CONST;

	if (ev->_major || 
		(!contained) ||
		(!somSelf))
	{
		RHBOPT_throw_StExcep(ev,INTERNAL,UnexpectedNULL,NO);
	}
	else
	{
		ORBData *somThis=ORBGetData(somSelf);
		RHBORB *c_orb=somThis->c_orb;

		RHBOPT_cleanup_push(orb_describe_contained_cleanup,&data);

		if (SOMObject_somIsA(contained,c_orb->somClass_SOMDClientProxy_ref))
		{
			NamedValue result_describe=RHBSOMD_Init_NamedValue;
			NamedValue result_mods=RHBSOMD_Init_NamedValue;
			NamedValue result_abs=RHBSOMD_Init_NamedValue;
			NamedValue result_id=RHBSOMD_Init_NamedValue;
#ifndef USE_APPLE_SOM
			char *sz__get_absolute_name=somStringFromId(
					c_orb->somId_get_absolute_name);
			char *sz__get_id=somStringFromId(
					c_orb->somId_get_id);
			char *sz_describe=somStringFromId(
					c_orb->somIddescribe);
			char *sz__get_somModifiers=somStringFromId(
					c_orb->somId_get_somModifiers);
#endif

			*flags=ORB_DESCRIBE_FREE;

			if (absolute_name && !ev->_major)
			{
				result_abs.argument._type=TC_string;
				result_abs.argument._value=absolute_name;

				SOMDClientProxy_create_request(contained,ev,NULL,
						sz__get_absolute_name,
						NULL,
						&result_abs,
						&data.req_abs,0);

#if defined(_WIN32) && defined(_DEBUG) && defined(_M_IX86)
				__asm int 3
#endif
			}

			if (id && !ev->_major)
			{
				result_id.argument._type=TC_string;
				result_id.argument._value=id;

				SOMDClientProxy_create_request(contained,ev,NULL,
						sz__get_id,
						NULL,
						&result_id,
						&data.req_id,0);
			}

			if (desc_value && !ev->_major)
			{
				result_describe.argument._type=somdTC_Contained_Description;
				result_describe.argument._value=&data.desc_CORBA_1_1;

				SOMDClientProxy_create_request(contained,ev,NULL,
						sz_describe,
						NULL,
						&result_describe,
						&data.req_describe,0);
			}

			if (mods && !ev->_major)
			{
				result_mods.argument._type=somdTC_sequence_somModifier;
				result_mods.argument._value=mods;

				SOMDClientProxy_create_request(contained,ev,NULL,
						sz__get_somModifiers,
						NULL,
						&result_mods,
						&data.req_mods,0);
			}

			if (data.req_mods && !ev->_major)
			{
				Request_send(data.req_mods,ev,0);
			}

			if (data.req_abs && !ev->_major)
			{
				Request_send(data.req_abs,ev,0);
			}

			if (data.req_describe && !ev->_major)
			{
				Request_send(data.req_describe,ev,0);
			}

			if (data.req_mods && !ev->_major)
			{
				Request_get_response(data.req_mods,ev,0);
			}

			if (data.req_describe && !ev->_major)
			{
				Request_get_response(data.req_describe,ev,0);
			}

			if (data.req_abs && !ev->_major)
			{
				Request_get_response(data.req_abs,ev,0);
			}

			if (data.req_id && !ev->_major)
			{
				Request_get_response(data.req_id,ev,0);
			}
		}
		else
		{
			*flags=ORB_DESCRIBE_SOMIR;

			if (mods && !ev->_major)
			{
/*				data.id_mods=somIdFromString(sz__get_somModifiers);*/

				somva_SOMObject_somDispatch(contained,
						(somToken)mods,
						c_orb->somId_get_somModifiers,
						contained,
						ev); /* data.id_mods */
			}

			if (desc_value && !ev->_major)
			{
/*				data.id_describe=somIdFromString(sz_describe); */

				somva_SOMObject_somDispatch(contained,
						(somToken)&data.desc_CORBA_1_1,
						c_orb->somIddescribe,
						contained,
						ev); /* data.id_describe */
			}

			if (absolute_name && !ev->_major)
			{
/*				data.id_abs=somIdFromString(sz__get_absolute_name);*/

				somva_SOMObject_somDispatch(contained,
						(somToken)absolute_name,
						c_orb->somId_get_absolute_name,
						contained,
						ev); /* data.id_abs */
			}

			if (id && !ev->_major)
			{
/*				data.id_id=somIdFromString(sz__get_id); */

				somva_SOMObject_somDispatch(contained,
						(somToken)id,
						c_orb->somId_get_id,
						contained,
						ev); /* data.id_id */
			}
		}

		if (desc_value && !ev->_major)
		{
			any *result=&data.desc_CORBA_1_1.value;

			*desc_value=*result;
			result->_type=NULL;
			result->_value=NULL;
		}

		RHBOPT_cleanup_pop();
	}
}

struct orb_lookup_id
{
	Repository SOMSTAR repository;
	somId mid;
	Request SOMSTAR request;
};

RHBOPT_cleanup_begin(orb_lookup_id_cleanup,pv)

	struct orb_lookup_id *data=pv;

	if (data->mid) SOMFree(data->mid);
	if (data->request) somReleaseObjectReference(data->request);
	if (data->repository) somReleaseObjectReference(data->repository);

RHBOPT_cleanup_end

SOM_Scope Contained SOMSTAR SOMLINK orb_lookup_id(
		ORB SOMSTAR somSelf,
		Environment *ev,
		Repository SOMSTAR repository_arg,
		char *id)
{
Repository SOMSTAR RHBOPT_volatile repository=repository_arg;
ORBData *somThis=ORBGetData(somSelf);
Contained SOMSTAR result=NULL;
struct orb_lookup_id data={NULL,NULL,NULL};
static char method[]="lookup_id";
NamedValue nv=RHBSOMD_Init_NamedValue;

	RHBOPT_ASSERT(id);
	RHBOPT_ASSERT(id[0]==':')
	RHBOPT_ASSERT(id[1]==':')

	RHBOPT_cleanup_push(orb_lookup_id_cleanup,&data);

	if (!ev->_major)
	{
		if (!repository)
		{
			repository=RHBORB_acquire_interface_repository(somThis->c_orb,ev,id);

			if (!ev->_major)
			{
				data.repository=repository;
			}
		}
	}

	if (repository && !ev->_major)
	{
		if (SOMObject_somIsA(repository,somThis->c_orb->somClass_SOMDClientProxy_ref))
		{
			nv.argument._type=TC_Object;
			nv.argument._value=&result;
				
			SOMDClientProxy_create_request(repository,ev,NULL,
						method,
						NULL,
						&nv,
						&data.request,0);

			if (data.request && !ev->_major)
			{
				Request_add_arg(data.request,ev,
						"id",
						TC_string,
						&id,
						0,
						ARG_IN);

				if (!ev->_major)
				{
					Request_invoke(data.request,ev,0);
				}
			}
		}
		else
		{
			data.mid=somIdFromString(method);

			somva_SOMObject_somDispatch(repository,(somToken)&result,data.mid,
					repository,ev,id);
		}
	}

	RHBOPT_cleanup_pop();

	return result;
}

struct orb_lookup_name
{
	somId id;
	Request SOMSTAR request;
};

RHBOPT_cleanup_begin(orb_lookup_name_cleanup,pv)

struct orb_lookup_name *data=pv;

	if (data->id)
	{
		SOMFree(data->id);
	}

	if (data->request)
	{
		somReleaseObjectReference(data->request);
	}

RHBOPT_cleanup_end

SOM_Scope _IDL_SEQUENCE_SOMObject SOMLINK orb_lookup_name(
				ORB SOMSTAR somSelf,
				Environment *ev,
				SOMObject SOMSTAR cnr,
				char * search_name,
				long levels_to_search,
				unsigned long limit_type,
				boolean exclude_inherited)
{
	_IDL_SEQUENCE_SOMObject result={0,0,NULL};
	struct orb_lookup_name data={NULL,NULL};
	NamedValue result_nv=RHBSOMD_Init_NamedValue;
	ORBData *somThis=ORBGetData(somSelf);
	char *op="lookup_name";
	char *somir_type="all";

	switch (limit_type)
	{
	case CORBA_dk_Operation:
		somir_type="OperationDef";
		break;
	case CORBA_dk_Interface:
		somir_type="InterfaceDef";
		break;
	case CORBA_dk_Exception:
		somir_type="ExceptionDef";
		break;
	}

	RHBOPT_cleanup_push(orb_lookup_name_cleanup,&data);

	if (SOMObject_somIsA(cnr,somThis->c_orb->somClass_SOMDClientProxy_ref))
	{
		result_nv.argument._type=somdTC_sequence_SOMObject;
		result_nv.argument._value=&result;

		if (!SOMDClientProxy_create_request(cnr,ev,
				NULL,
				op,
				NULL,
				&result_nv,
				&data.request,
				0))
		{
			Request_add_arg(data.request,ev,"search_name",TC_string,&search_name,0,ARG_IN);
			Request_add_arg(data.request,ev,"levels_to_search",TC_long,&levels_to_search,0,ARG_IN);
			Request_add_arg(data.request,ev,"limit_type",TC_string,&somir_type,0,ARG_IN);
			Request_add_arg(data.request,ev,"exclude_inherited",TC_boolean,&exclude_inherited,0,ARG_IN);

			Request_invoke(data.request,ev,0);
		}
	}
	else
	{
		data.id=somIdFromString(op);

		somva_SOMObject_somDispatch(cnr,
				(somToken *)(void *)&result,data.id,
				cnr,ev,search_name,levels_to_search,
				somir_type,
				exclude_inherited);
	}

	RHBOPT_cleanup_pop();

	if (ev->_major)
	{
		_IDL_SEQUENCE_SOMObject resultz={0,0,NULL};
		return resultz;
	}

	return result;
}

