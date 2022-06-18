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

#define SOMDServer_VA_STUBS
#define SOMDServer_VA_EXTERN

#include <rhbopt.h>
#include <rhbsomd.h>

#ifdef _PLATFORM_MACINTOSH_
	#include <Errors.h>
#endif

#ifndef SOM_Module_somdserv_Source
#define SOM_Module_somdserv_Source
#endif
#define SOMDServer_Class_Source

#include "somdserv.ih"

SOM_Scope SOMDObject SOMSTAR  SOMLINK somdserv_somdRefFromSOMObj(
		SOMDServer SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR somobj)
{
	SOMDServerData *somThis=SOMDServerGetData(somSelf);

#ifdef _DEBUG
	if (somThis->ImplDefObject != SOMD_ImplDefObject)
	{
		SOMD_bomb("wrong impldef");
	}
#endif

    return SOMOA_create_SOM_ref(
			SOMD_SOMOAObject,
			ev,
			somobj,
			somThis->ImplDefObject);
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somdserv_somdSOMObjFromRef(
		SOMDServer SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR objref)
{
	RHBOPT_unused(somSelf)

    return SOMOA_get_SOM_object(SOMD_SOMOAObject,ev,objref);
}

SOM_Scope void  SOMLINK somdserv_somdDispatchMethod(
		SOMDServer SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR somobj, 
		somToken* retValue, 
		somId methodId, 
		va_list ap)
{
	RHBOPT_unused(somSelf)

	if (somobj)
	{
		if (!SOMObject_somDispatch(somobj,retValue,methodId,ap))
		{
			RHBOPT_throw_StExcep(ev,
				OBJ_ADAPTER,DispatchError,NO);
		}
	}
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somdserv_somdCreateObj(
		SOMDServer SOMSTAR somSelf, 
		Environment *ev, 
		Identifier objclass, 
		corbastring hints)
{
	SOMClass SOMSTAR cls=NULL;

	RHBOPT_unused(hints)

	cls=SOMDServer_somdGetClassObj(somSelf,ev,objclass);

	if (cls) 
	{
		SOMObject SOMSTAR obj=SOMClass_somNew(cls);

		somReleaseClassReference(cls);

		return obj;
	}

#ifdef _DEBUG
	somPrintf("error: SOMDServer::somdCreateObj(%s,%s)\n",
		objclass ? objclass : "",
		hints ? hints : "");
#endif

	RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES)

    return 0;
}

SOM_Scope void  SOMLINK somdserv_somdDeleteObj(
		SOMDServer SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR somobj)
{
	if (somobj)
	{
		RHBServerRequest *request;

		if (somobj==somSelf)
		{
			if (SOMD_SOMOAObject)
			{
				SOMOA_interrupt_server(SOMD_SOMOAObject,ev);
			}
			else
			{
				RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,BadObjref,YES)
			}
		}
		else
		{
			RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

			if (thread)
			{
				request=thread->server_request;

				if (request)
				{
					RHBImplementationDef_remove_client_ref(request->impl,
						somobj,0);
				}
			}

			somReleaseObjectReference(somobj);
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,YES)
	}
}

struct somdserv_somdGetClassObj
{
	SOMClassMgr SOMSTAR clsMgr;
	somId id;
};

RHBOPT_cleanup_begin(somdserv_somdGetClassObj_cleanup,pv)

struct somdserv_somdGetClassObj *data=pv;

	if (data->id) SOMFree(data->id);
	if (data->clsMgr) somReleaseClassManagerReference(data->clsMgr);

RHBOPT_cleanup_end

SOM_Scope SOMClass SOMSTAR  SOMLINK somdserv_somdGetClassObj(
		SOMDServer SOMSTAR somSelf, 
		Environment *ev, 
		Identifier objclass)
{
	SOMClass SOMSTAR RHBOPT_volatile cls=NULL;
	struct somdserv_somdGetClassObj data={NULL,NULL};

	RHBOPT_cleanup_push(somdserv_somdGetClassObj_cleanup,&data);

	data.id=somIdFromString(objclass);

	if (data.id)
	{
		data.clsMgr=somGetClassManagerReference();

		if (data.clsMgr)
		{
		    cls=SOMClassMgr_somFindClass(data.clsMgr,data.id,0,0);
		}
	}

	RHBOPT_unused(somSelf)

	if (!cls)
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES)
	}

	RHBOPT_cleanup_pop();

	return cls;
}

SOM_Scope boolean  SOMLINK somdserv_somdObjReferencesCached(
		SOMDServer SOMSTAR somSelf, 
		Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

    return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK somdserv_somdCreateFactory(
	SOMDServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring className,
	/* in */ ExtendedNaming_PropertyList *props)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(className)
	RHBOPT_unused(props)

	RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES)

	return 0;
}

#ifdef SOMDServer_BeginInitializer_somDefaultInit
SOM_Scope void SOMLINK somdserv_somDefaultInit(SOMDServer SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	SOMDServerData *somThis;
	somBooleanVector myMask;

	SOMDServer_BeginInitializer_somDefaultInit

	somThis->ImplDefObject=NULL;

	SOMDServer_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}
#else
SOM_Scope void SOMLINK somdserv_somInit(SOMDServer SOMSTAR somSelf)
{
#ifdef SOMDServer_parent_SOMRefObject_somInit
	SOMDServer_parent_SOMRefObject_somInit(somSelf);
#else
	SOMDServer_parent_SOMObject_somInit(somSelf);
#endif
    SOMDServerGetData(somSelf)->ImplDefObject=NULL;
}
#endif

#ifdef SOMDServer_BeginDestructor
SOM_Scope void SOMLINK somdserv_somDestruct(SOMDServer SOMSTAR somSelf,
											boolean doFree,
											somDestructCtrl *ctrl)
{
	SOMDServerData *somThis;
	somBooleanVector myMask;
	somDestructCtrl globalCtrl;
	SOMDServer_BeginDestructor
#else
SOM_Scope void SOMLINK somdserv_somUninit(SOMDServer SOMSTAR somSelf)
{
	SOMDServerData *somThis=SOMDServerGetData(somSelf);
#endif

	if (somThis->ImplDefObject)
	{
		ImplementationDef SOMSTAR idef=somThis->ImplDefObject;
		somThis->ImplDefObject=NULL;
		somReleaseObjectReference(idef);
	}

	if (SOMD_ServerObject==somSelf)
	{
		SOMD_ServerObject=NULL;
	}

	if (SOMMSingleInstanceClassData.sommSingleInstanceFreed)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		SOMMSingleInstance_sommSingleInstanceFreed(somSelf->mtab->classObject,&ev,somSelf);
		SOM_UninitEnvironment(&ev);
	}

#ifdef SOMDServer_EndDestructor
	SOMDServer_EndDestructor
#else
	#ifdef SOMDServer_parent_SOMRefObject_somUninit
		SOMDServer_parent_SOMRefObject_somUninit(somSelf);
	#else
		SOMDServer_parent_SOMObject_somUninit(somSelf);
	#endif
#endif
}

SOM_Scope void SOMLINK somdserv__set_ImplDefObject(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			ImplementationDef SOMSTAR ImplDefObject)
{
	SOMDServerData *somThis=SOMDServerGetData(somSelf);
	ImplementationDef SOMSTAR old_ref=somThis->ImplDefObject;

	if (old_ref != ImplDefObject)
	{
		if (ImplDefObject) 
		{
			ImplDefObject=ImplementationDef_somDuplicateReference(ImplDefObject);
		}

		somThis->ImplDefObject=ImplDefObject;

		if (old_ref) somReleaseObjectReference(old_ref);
	}

	RHBOPT_unused(ev)
}

SOM_Scope ImplementationDef SOMSTAR SOMLINK somdserv__get_ImplDefObject(
			SOMDServer SOMSTAR somSelf,
			Environment *ev)
{
    SOMDServerData *somThis=SOMDServerGetData(somSelf);
	ImplementationDef SOMSTAR impl;
	RHBOPT_unused(ev)

	RHBORB_guard(0)

	impl=somThis->ImplDefObject;

	if (impl) impl=ImplementationDef_somDuplicateReference(impl);

	RHBORB_unguard(0)

	return impl;
}

#ifdef SOMDServer_test_context
SOM_Scope void SOMLINK somdserv_test_context(SOMDServer SOMSTAR somSelf,Environment *ev,Context SOMSTAR ctx,char *param)
{
	dump_somPrintf("somdserv_test_context(%s)\n",param);
	Context_somDumpSelf(ctx,0);
}
#endif

#ifdef _WIN32
	#ifndef sleep
		#define sleep(x)    Sleep(x * 1000)
	#endif
#endif

struct somdserv_somdDispatchArgs
{
	somVaBuf vabuf;
	char *method_str;
};

RHBOPT_cleanup_begin(somdserv_somdDispatchArgs_cleanup,pv)

	struct somdserv_somdDispatchArgs *data=pv;

	if (data->vabuf)
	{
		somVaBuf_destroy(data->vabuf);
	}
#ifdef USE_APPLE_SOM
	if (data->method_str)
	{
		SOMFree(data->method_str);
	}
#endif

RHBOPT_cleanup_end

SOM_Scope void SOMLINK somdserv_somdDispatchArgs(
	SOMDServer SOMSTAR somSelf,
	Environment *ev,
	Context SOMSTAR ctx,
	/* in */ SOMObject SOMSTAR target,
	/* in */ somId method,
	/* inout */ any *result,
	/* in */ NVList SOMSTAR args)
{
struct somdserv_somdDispatchArgs data={NULL,NULL};

	data.vabuf=somVaBuf_create(NULL,0);

	RHBOPT_cleanup_push(somdserv_somdDispatchArgs_cleanup,&data);

	if (data.vabuf)
	{
#ifdef USE_APPLE_SOM
		data.method_str=somMakeStringFromId(method);
#else
		data.method_str=somStringFromId(method);
#endif

		if (data.method_str)
		{
			RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);
			long count=0;
			va_list ap;
			somToken *retVal=(somToken *)result->_value;
			somVaBuf_add(data.vabuf,&target,tk_pointer);

/*			somPrintf("somdDispatch(%s)\n",data.method_str); */

			if (!RHBORB_is_method_oidl(orb,target,data.method_str))
			{
				somVaBuf_add(data.vabuf,&ev,tk_pointer);
			}

			if (ctx)
			{
				somVaBuf_add(data.vabuf,&ctx,tk_pointer);
			}

			if (args)
			{
				if (!NVList_get_count(args,ev,&count))
				{
					if (count)
					{
						long l=0;

						while (l < count)
						{
							NamedValue nv=RHBSOMD_Init_NamedValue;
							boolean byRef=0;

							if (NVList_get_item(args,ev,
									l++,
									&nv.name,
									&nv.argument._type,
									&nv.argument._value,
									&nv.len,
									&nv.arg_modes)) break;

							if (nv.arg_modes & ARG_IN)
							{
								void *vp=nv.argument._value;
								TCKind kind=TypeCode_kind(nv.argument._type,ev);

								switch (kind)
								{
								case tk_char:
								case tk_octet:
								case tk_boolean:
								case tk_ushort:
								case tk_short:
								case tk_long:
								case tk_ulong:
								case tk_enum:
								case tk_float:
								case tk_double:
								case tk_string:
									somVaBuf_add(data.vabuf,vp,kind);
									break;

								case tk_objref:
									somVaBuf_add(data.vabuf,vp,tk_pointer);
									break;

								default:
							/*		somPrintf("In by reference... ");
									tcPrint(args._buffer[l].argument._type,ev);*/
									byRef=1;
								}
							}
							else
							{
								byRef=1;
							}

							if (byRef)
							{
								/* argument._value is just a void * to the actual data */
								somVaBuf_add(data.vabuf,
										&nv.argument._value,
										tk_pointer);
							}
						}
					}
				}
			}

			somVaBuf_get_valist(data.vabuf,&ap);

			if (!retVal)
			{
				switch (TypeCode_kind(result->_type,ev))
				{
				case tk_array:
					/* special case for arrays */
					retVal=(somToken *)&(result->_value);
					break;
				default:
					break;
				}
			}

			SOMDServer_somdDispatchMethod(somSelf,ev,target,retVal,method,ap);
		}
		else
		{
			RHBOPT_throw_StExcep(ev,BAD_OPERATION,BadMethodName,NO);
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,NO);
	}

	RHBOPT_cleanup_pop();
}

