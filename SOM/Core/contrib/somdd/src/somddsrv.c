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

#ifdef _WIN32
	#define SOMDLLEXPORT
#endif

#define SOM_Module_somddsrv_Source
#define SOMDD_Server_Class_Source
#define SOMDD_ServerMgr_Class_Source
#define SOM_Module_implreps_Source

#include <rhbsomdd.h>
/*#include <intfacdf.h> */
#include <rhbsomuk.h>
#include <implreps.h>
#include <poa.h>
#include <somddsrv.ih>

#define SOMDD_type_SOMDServer				1
#define SOMDD_type_ImplRepository			2
#define SOMDD_type_ImplementationDef		3
#define SOMDD_type_SOMClass					4
#define SOMDD_type_SOMDServerMgr			5

static char *SOMDD_get_objects_type_id(SOMDObject SOMSTAR dobj,Environment *ev);

SOMDD_DEFMSG(SOMDD_LOGMSG_DISPATCH,WARN,
			 SOMDDMSG_SOMDDDispatchEx,
			 "exception %1 thrown during message dispatch of %2::%3")

static ReferenceData SOMDD_create_ref_data(unsigned long type,char *name)
{
	ReferenceData data={0,0,NULL};
	octet *op;
	int i=0;

	if (name) i=(int)strlen(name)+1;

	data._length=sizeof(type)+i;

	data._maximum=data._length;

	data._buffer=SOMMalloc(data._length);

	if (!data._buffer)
	{
		data._length=0;
		data._maximum=0;
		return data;
	}

	op=data._buffer;
	memcpy(op,&type,sizeof(type));
	op+=sizeof(type);
	if (i) memcpy(op,name,i);

	return data;
}

static unsigned long SOMDD_unpack_ref_data(ReferenceData *data,char **phint)
{
	unsigned long ul=0;

	if (data->_length >= sizeof(ul)) 
	{
		unsigned long *ulp=(unsigned long *)data->_buffer;

		ul=*ulp++;

		if (data->_length > sizeof(ul)) 
		{
			*phint=(char *)ulp;
		}
	}
	else
	{
		ul=SOMDD_type_SOMDServer;
	}

	return ul;
}

struct Repository_lookup_id_data { somId id; };

RHBOPT_cleanup_begin(Repository_lookup_id_cleanup,pv)

	struct Repository_lookup_id_data *data=pv;
	if (data->id) SOMFree(data->id);

RHBOPT_cleanup_end

Contained SOMSTAR SOMLINK Repository_lookup_id(
		Repository SOMSTAR rep,Environment *ev,char *str)
{
	Contained SOMSTAR cnd=NULL;
	struct Repository_lookup_id_data data;

	data.id=somIdFromString("lookup_id");

	RHBOPT_cleanup_push(Repository_lookup_id_cleanup,&data);

	if (data.id)
	{
		somva_SOMObject_somDispatch(rep,(somToken *)(void *)&cnd,data.id,
				rep,ev,str);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
	}

	RHBOPT_cleanup_pop();

	return cnd;
}

struct Contained__get_id_data 
{ 
	somId id; 
	SOMClass SOMSTAR somClassSOMDObject;
};

RHBOPT_cleanup_begin(Contained__get_id_cleanup,pv)

	struct Contained__get_id_data *data=pv;

	if (data->id) 
	{
		SOMFree(data->id);
	}

	if (data->somClassSOMDObject) 
	{
		somReleaseClassReference(data->somClassSOMDObject);
	}

RHBOPT_cleanup_end

char * SOMLINK Contained__get_id(Contained SOMSTAR cnd,Environment *ev)
{
	char *result=NULL;
	struct Contained__get_id_data data={NULL,NULL};

	data.id=somIdFromString("_get_id");

	RHBOPT_cleanup_push(Contained__get_id_cleanup,&data);

	if (data.id)
	{
		boolean doDuplicate=0;

		data.somClassSOMDObject=somNewClassReference(SOMDObject);

		doDuplicate=(boolean)(SOMObject_somIsA(cnd,data.somClassSOMDObject) ? 0 : 1);

		somva_SOMObject_somDispatch(cnd,(somToken *)(void *)&result,data.id,cnd,ev);

		if (doDuplicate && result)
		{
			somId desc=SOMClass_somGetMethodDescriptor(
				SOMObject_somGetClass(cnd),
				data.id);

			if (desc)
			{
				const char *p=somStringFromId(desc);

				if (p && strcmp(p,"Contained::_get_id"))
				{
					doDuplicate=0;
				}
			}

			if (doDuplicate)
			{
				result=SOMDD_dupl_string(result);
			}
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
	}

	RHBOPT_cleanup_pop();

	return result;
}

SOM_Scope void SOMLINK somddsrv_somInit(
	SOMDD_Server SOMSTAR somSelf)
{
	SOMDD_ServerData *somThis=SOMDD_ServerGetData(somSelf);
	SOMDD_Server_parent_SOMDServer_somInit(somSelf);

	somThis->SOMClassClassObject=somNewClassReference(SOMClass);
	somThis->ImplementationDefClassObject=somNewClassReference(ImplementationDef);
	somThis->SOMDServerMgrClassObject=somNewClassReference(SOMDServerMgr);
	somThis->DynamicImplClassObject=somNewClassReference(PortableServer_DynamicImpl);
}

SOM_Scope void SOMLINK somddsrv_somUninit(
	SOMDD_Server SOMSTAR somSelf)
{
	SOMDD_ServerData *somThis=SOMDD_ServerGetData(somSelf);

#define ZAP_CLASSREF(x) if (x) { somReleaseClassReference(x); x=0; }

	ZAP_CLASSREF(somThis->ImplementationDefClassObject);
	ZAP_CLASSREF(somThis->SOMClassClassObject);
	ZAP_CLASSREF(somThis->SOMDServerMgrClassObject);
	ZAP_CLASSREF(somThis->DynamicImplClassObject);

#undef ZAP_CLASSREF

	SOMDD_Server_parent_SOMDServer_somUninit(somSelf);
}

struct somddsrv_somdCreateObj
{
	somId idClass,idMethod;
	SOMClass SOMSTAR classObject;
	SOMClassMgr SOMSTAR classMgr;
	SOMObject SOMSTAR repository;
};

RHBOPT_cleanup_begin(somddsrv_somdCreateObj_cleanup,pv)

struct somddsrv_somdCreateObj *data=pv;

	if (data->idClass) 
	{
		SOMFree(data->idClass);
	}

	if (data->idMethod) 
	{
		SOMFree(data->idMethod);
	}

	if (data->classObject)
	{
		somReleaseClassReference(data->classObject);
	}

	if (data->classMgr)
	{
		somReleaseClassManagerReference(data->classMgr);
	}

	if (data->repository)
	{
		somReleaseObjectReference(data->repository);
	}

RHBOPT_cleanup_end

#define create_func(x) static SOMObject SOMSTAR somddsrv_somdCreateObj_##x(  \
		struct somddsrv_somdCreateObj *data,	\
		SOMDD_Server SOMSTAR somSelf,Environment *ev, \
		char *objclass, \
		char *hints) 

create_func(parent)
{
	return SOMDD_Server_parent_SOMDServer_somdCreateObj(
			somSelf,ev,objclass,hints);
}

create_func(ImplRepository)
{
	if (SOMD_ImplRepObject)
	{
		return ImplRepository_somDuplicateReference(SOMD_ImplRepObject);
	}
	return NULL;
}

create_func(SOMDServerMgr)
{
#ifdef somNewClassReference
	data->classObject=somNewClassReference(SOMDD_ServerMgr);
#else
	data->classObject=_SOMDD_ServerMgrNew;
#endif
	if (data->classObject) return SOMClass_somNew(data->classObject);

	return NULL;
}

create_func(SOMObject)
{
	SOMObject SOMSTAR result=NULL;

	data->idClass=somIdFromString(objclass);
	data->classMgr=somGetClassManagerReference();

	if (data->idClass && data->classMgr)
	{
		data->classObject=SOMClassMgr_somFindClass(data->classMgr,data->idClass,0,0);

		if (data->classObject && !ev->_major)
		{
			result=SOMClass_somNew(data->classObject);
		}
	}

	return result;
}

static struct somddsrv_somdCreateObj_map {
	char *objclass;
	SOMObject SOMSTAR (*func)(struct somddsrv_somdCreateObj *data,
			SOMDD_Server SOMSTAR somSelf,
			Environment *ev,
			char *className,
			char *hints);
}

somddsrv_somdCreateObj_map[]=
{
	{"SOMDServerMgr",somddsrv_somdCreateObj_SOMDServerMgr},
	{"ImplementationDef",somddsrv_somdCreateObj_SOMObject},
	{"SOMOA",somddsrv_somdCreateObj_SOMObject},
	{"somStream::MemoryStreamIO",somddsrv_somdCreateObj_SOMObject},
	{"ImplRepository",somddsrv_somdCreateObj_ImplRepository},
	{"SOMDObjectMgr",somddsrv_somdCreateObj_parent}
};

SOM_Scope SOMObject SOMSTAR SOMLINK somddsrv_somdCreateObj(
	SOMDD_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ Identifier objclass,
	/* in */ corbastring hints)
{
/*SOMDD_ServerData *somThis=SOMDD_ServerGetData(somSelf);*/
struct somddsrv_somdCreateObj data={NULL,NULL,NULL,NULL,NULL};
SOMObject SOMSTAR RHBOPT_volatile result=NULL;
boolean RHBOPT_volatile handled=0;
size_t RHBOPT_volatile i=sizeof(somddsrv_somdCreateObj_map)/sizeof(somddsrv_somdCreateObj_map[0]);

	if (ev->_major) return NULL;

	if ((!objclass) || !objclass[0])
	{
		RHBOPT_throw_StExcep(ev,INV_IDENT,UnexpectedNULL,NO);
		return NULL;
	}

	RHBOPT_cleanup_push(somddsrv_somdCreateObj_cleanup,&data);

	while (i--)
	{
		if (!strcmp(objclass,somddsrv_somdCreateObj_map[i].objclass))
		{
			handled=1;
			result=somddsrv_somdCreateObj_map[i].func(&data,somSelf,ev,objclass,hints);
			break;
		}
	}

	if (!handled)
	{
		somPrintf("somdCreateObj(%s,%s)\n",objclass,hints ? hints : "");

		RHBOPT_throw_StExcep(ev,NO_PERMISSION,AccessDenied,NO);
	}

	if ((!result) && (!ev->_major) && !handled)
	{
		data.idClass=somIdFromString(objclass);
		data.classMgr=somGetClassManagerReference();

		if (data.idClass && data.classMgr)
		{
			data.classObject=SOMClassMgr_somFindClass(
				data.classMgr,data.idClass,0,0);

			if (data.classObject && !ev->_major)
			{
				result=SOMClass_somNew(data.classObject);

				if (!result)
				{
					RHBOPT_throw_StExcep(ev,NO_MEMORY,UnexpectedNULL,NO);
				}
			}
		}

		if (!data.classObject)
		{
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
		}
	}

	RHBOPT_cleanup_pop();

	if ((!result) && !ev->_major)
	{
		if (!handled)
		{
			result=SOMDD_Server_parent_SOMDServer_somdCreateObj(
				somSelf,ev,objclass,hints);
		}
		else
		{
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
		}
	}

	return result;
}

struct somddsrv_somdDispatchMethod
{
	char *method;
	char *className;
};

#ifdef USE_APPLE_SOM
RHBOPT_cleanup_begin(somddsrv_somdDispatchMethod_cleanup,pv)

struct somddsrv_somdDispatchMethod *data=pv;

	if (data->method) SOMFree(data->method);

RHBOPT_cleanup_end
#endif

SOM_Scope void SOMLINK somddsrv_somdDispatchMethod(
	SOMDD_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR somobj,
	/* out */ somToken *retValue,
	/* in */ somId methodId,
	/* in */ va_list ap)
{
#if defined(_WIN32)&&defined(_DEBUG)
/*	somPrintf(
			"SOMDServer::somdDispatchMethod(%s->%s)\n",
				somobj->mtab->className,*methodId);*/
#endif

	struct somddsrv_somdDispatchMethod data={NULL,NULL};
#ifdef USE_APPLE_SOM
	RHBOPT_cleanup_push(somddsrv_somdDispatchMethod_cleanup,&data);
	data.method=somMakeStringFromId(methodId);
#else
	data.method=somStringFromId(methodId);
#endif
	data.className=SOMObject_somGetClassName(somobj);

	SOMDD_Server_parent_SOMDServer_somdDispatchMethod(
			somSelf,ev,somobj,retValue,methodId,ap);

	if (ev->_major)
	{
		SOMDD_syslog(&SOMDD_LOGMSG_DISPATCH,
				somExceptionId(ev),
				data.className,
				data.method,
				NULL);
	}

#ifdef USE_APPLE_SOM
	RHBOPT_cleanup_pop();
#endif
}

static Repository SOMSTAR get_repository(Environment *ev)
{
	return ORB_resolve_initial_references(SOMD_ORBObject,ev,"InterfaceRepository");
}

struct somddsrv_somdRefFromSOMObj_data
{
	ReferenceData refdata;
	ImplementationDef SOMSTAR thisImpl;
	ImplementationDef SOMSTAR idef;
	InterfaceDef SOMSTAR iface;
	SOMClassMgr SOMSTAR mgr;
	Repository SOMSTAR rep;
	char *id;
};

RHBOPT_cleanup_begin(somddsrv_somdRefFromSOMObj_cleanup,pv)

	struct somddsrv_somdRefFromSOMObj_data *data=pv;

	if (data->thisImpl) somReleaseObjectReference(data->thisImpl);
	if (data->rep) somReleaseObjectReference(data->rep);
	if (data->iface) somReleaseObjectReference(data->iface);
	if (data->idef) somReleaseObjectReference(data->idef);
	if (data->refdata._buffer) SOMFree(data->refdata._buffer);
	if (data->id) SOMFree(data->id);
	if (data->mgr) somReleaseClassManagerReference(data->mgr);

RHBOPT_cleanup_end

#define SOMDD_CLEAR_OBJECT(x)  if (x) { SOMObject SOMSTAR y=x; x=NULL; somReleaseObjectReference(y); }

SOM_Scope SOMDObject SOMSTAR SOMLINK somddsrv_somdRefFromSOMObj(
	SOMDD_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR somobj)
{
	SOMDObject SOMSTAR RHBOPT_volatile retVal=NULL;

	if (!ev->_major)
	{
		SOMDD_ServerData *somThis=SOMDD_ServerGetData(somSelf);
		struct somddsrv_somdRefFromSOMObj_data data={{0,0,NULL},NULL,NULL,NULL,NULL,NULL,NULL};
		RHBOPT_cleanup_push(somddsrv_somdRefFromSOMObj_cleanup,&data);

		data.thisImpl=SOMDD_Server__get_ImplDefObject(somSelf,ev);

		if (ev->_major)
		{
			data.thisImpl=NULL;
		}
		else
		{
			if (!data.thisImpl)
			{
				RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,MAYBE);
			}
		}

		if ((somobj==somSelf) && (!ev->_major))
		{
			data.rep=get_repository(ev);

			if (data.rep && !ev->_major)
			{
				data.iface=Repository_lookup_id(data.rep,ev,"::SOMDServer");

				if (data.iface && !ev->_major)
				{
					data.refdata=SOMDD_create_ref_data(SOMDD_type_SOMDServer,0);

					retVal=SOMOA_create_constant(
							SOMD_SOMOAObject,
							ev,
							&data.refdata,
							data.iface,
							data.thisImpl);

					SOMDD_CLEAR_OBJECT(data.iface);
				}
			}
		}

		if ((!retVal) && (!ev->_major))
		{
			if (SOMObject_somIsA(somobj,somThis->ImplementationDefClassObject))
			{
				data.id=ImplementationDef__get_impl_id(somobj,ev);

				if (data.id && !ev->_major) 
				{
					if (data.id[0])
					{
						data.idef=ImplRepository_find_impldef(SOMD_ImplRepObject,ev,data.id);

						if (!data.idef)
						{
		/*					somPrintf("%s not in SOMD_ImplRepObject\n",p);*/

							SOMFree(data.id);
							data.id=NULL;

							somExceptionFree(ev);

							retVal=SOMDD_Server_parent_SOMDServer_somdRefFromSOMObj(
									somSelf,ev,somobj);
						}
						else
						{
							if (data.idef != somobj)
							{
			/*					somPrintf("%s not same as in SOMD_ImplRepObject\n",p);*/

								somExceptionFree(ev);

								retVal=SOMDD_Server_parent_SOMDServer_somdRefFromSOMObj(
										somSelf,ev,somobj);
							}
						}

						if ((!retVal) && (!ev->_major))
						{
							if (!data.rep)
							{
								data.rep=get_repository(ev);
							}

							if (data.rep)
							{
								SOMDD_CLEAR_OBJECT(data.iface);

								data.iface=Repository_lookup_id(data.rep,ev,"::ImplementationDef");

								if (data.iface && !ev->_major)
								{
									data.refdata=SOMDD_create_ref_data(SOMDD_type_ImplementationDef,data.id);

									retVal=SOMOA_create_constant(
											SOMD_SOMOAObject,
											ev,
											&data.refdata,
											data.iface,
											data.thisImpl);

									if (data.refdata._buffer) 
									{
										SOMFree(data.refdata._buffer);
										data.refdata._buffer=NULL;
									}
								}
							}
						}
					}
				}
			}
		}

		if ((!retVal) && (!ev->_major))
		{
			if (SOMObject_somIsA(somobj,somThis->SOMClassClassObject))
			{
				data.mgr=somGetClassManagerReference();

				if (data.mgr)
				{
					char *className=SOMClass_somGetName(somobj);
					SOMDD_CLEAR_OBJECT(data.iface);

					if (!data.rep) data.rep=get_repository(ev);

					if (data.rep && (!ev->_major))
					{
						data.iface=Repository_lookup_id(data.rep,ev,"::SOMClass");
					}

					if (data.iface && (!ev->_major))
					{
						data.refdata=SOMDD_create_ref_data(SOMDD_type_SOMClass,className);

						retVal=SOMOA_create_constant(
								SOMD_SOMOAObject,
								ev,
								&data.refdata,
								data.iface,
								data.thisImpl);

						if (data.refdata._buffer) 
						{
							SOMFree(data.refdata._buffer);
							data.refdata._buffer=NULL;
						}
					}
				}
			}
		}

		if ((!retVal) && (!ev->_major))
		{
			if (somobj==SOMD_ImplRepObject)
			{
				if (!data.rep) data.rep=get_repository(ev);

				if (data.rep && !ev->_major)
				{
					SOMDD_CLEAR_OBJECT(data.iface);

					data.iface=Repository_lookup_id(data.rep,ev,"::ImplRepository");

					if (data.iface && !ev->_major)
					{
						data.refdata=SOMDD_create_ref_data(SOMDD_type_ImplRepository,0);

						retVal=SOMOA_create_constant(
							SOMD_SOMOAObject,ev,
							&data.refdata,data.iface,data.thisImpl);
					}
				}
			}
		}

		if ((!retVal) && (!ev->_major))
		{
			if (SOMObject_somIsA(somobj,somThis->SOMDServerMgrClassObject))
			{
				if (!data.rep) data.rep=get_repository(ev);
				
				if (data.rep && !ev->_major)
				{
					SOMDD_CLEAR_OBJECT(data.iface);

					data.iface=Repository_lookup_id(data.rep,ev,"::SOMDServerMgr");

					if (data.iface && !ev->_major)
					{
						data.refdata=SOMDD_create_ref_data(SOMDD_type_SOMDServerMgr,0);

						retVal=SOMOA_create_constant(
								SOMD_SOMOAObject,
								ev,
								&data.refdata,
								data.iface,
								data.thisImpl);
					}
				}
			}
		}

		RHBOPT_cleanup_pop();
	}

	if (ev->_major) return NULL;
	if (retVal) return retVal;

	return SOMDD_Server_parent_SOMDServer_somdRefFromSOMObj(somSelf,ev,somobj);
}

struct somddsrv_somdSOMObjFromRef_data
{
	ImplementationDef SOMSTAR thisImplDef;
	ImplementationDef SOMSTAR other_idef;
	ImplementationDef SOMSTAR active_impldef;
	Repository SOMSTAR rep;
	SOMObject SOMSTAR newobj;
	SOMClassMgr SOMSTAR clsMgr;
	char *impl_id;
	char *other_idef_id;
	char *type_id;
	char *rep_id;
	somId sid;
	ReferenceData refdata;
	_IDL_SEQUENCE_octet seq;
};

RHBOPT_cleanup_begin(somddsrv_somdSOMObjFromRef_cleanup,pv)

struct somddsrv_somdSOMObjFromRef_data *locals=pv;

	if (locals->thisImplDef) somReleaseObjectReference(locals->thisImplDef);
	if (locals->other_idef) somReleaseObjectReference(locals->other_idef);
	if (locals->active_impldef) somReleaseObjectReference(locals->active_impldef);
	if (locals->rep) somReleaseObjectReference(locals->rep);
	if (locals->clsMgr) somReleaseClassManagerReference(locals->clsMgr);
	if (locals->newobj) somReleaseClassManagerReference(locals->newobj);
	if (locals->impl_id) SOMFree(locals->impl_id);
	if (locals->refdata._buffer) SOMFree(locals->refdata._buffer);
	if (locals->other_idef_id) SOMFree(locals->other_idef_id);
	if (locals->seq._buffer) SOMFree(locals->seq._buffer);
	if (locals->type_id) SOMFree(locals->type_id);
	if (locals->rep_id) SOMFree(locals->rep_id);
	if (locals->sid) SOMFree(locals->sid);

RHBOPT_cleanup_end

SOM_Scope SOMObject SOMSTAR SOMLINK somddsrv_somdSOMObjFromRef(
	SOMDD_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMDObject SOMSTAR objref)
{
SOMDD_ServerData *somThis=SOMDD_ServerGetData(somSelf);
struct somddsrv_somdSOMObjFromRef_data locals={
		NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,NULL,
		NULL,NULL,NULL,
		{0,0,NULL},
		{0,0,NULL}};
SOMObject SOMSTAR RHBOPT_volatile somobj=NULL;
	
	if (ev->_major)
	{
		return NULL;
	}

	if (!objref) 
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,MAYBE);

		return NULL;
	}

	RHBOPT_cleanup_push(somddsrv_somdSOMObjFromRef_cleanup,&locals);

	locals.thisImplDef=SOMDD_Server__get_ImplDefObject(somSelf,ev);

	if ((!ev->_major) && !locals.thisImplDef)
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,MAYBE);
	}

	if (!ev->_major)
	{
		locals.refdata=SOMOA_get_id(SOMD_SOMOAObject,ev,objref);

		/* check to see if it belongs to this process... */

		if (!ev->_major)
		{
			locals.impl_id=ImplementationDef__get_impl_id(locals.thisImplDef,ev);
		}
	
		if ((!ev->_major) && locals.impl_id)
		{
			locals.other_idef=SOMDObject_get_implementation(objref,ev);

			if ((!ev->_major) && (locals.other_idef))
			{
				locals.other_idef_id=ImplementationDef__get_impl_id(locals.other_idef,ev);

				if ((!ev->_major) && locals.other_idef_id)
				{
					if (strcmp(locals.impl_id,locals.other_idef_id))
					{
/*						somPrintf("it belongs in a different server...\n");*/

						locals.active_impldef=ImplRepositoryServer_get_active_impldef(
								SOMD_ImplRepObject,
								ev,
								locals.other_idef_id,
								objref);

						if ((!ev->_major) && locals.active_impldef)
						{
							if (SOMObject_somIsA(locals.active_impldef,
									somThis->DynamicImplClassObject))
							{
								locals.newobj=locals.active_impldef;
								locals.active_impldef=NULL;
							}
							else
							{
								boolean is_the_srvobj=1;

								if (locals.refdata._length)
								{
									is_the_srvobj=0;
								}

								if (!ev->_major)
								{
									if (is_the_srvobj)
									{
										_IDL_SEQUENCE_octet *seqp=
											ImplementationDef__get_svr_objref(locals.active_impldef,ev);

										if (!seqp)
										{
											RHBOPT_throw_StExcep(ev,INV_OBJREF,UnexpectedNULL,NO);
										}
										else
										{
											locals.seq._length=seqp->_length;
											locals.seq._maximum=locals.seq._length;
											locals.seq._buffer=SOMMalloc(locals.seq._length);
											memcpy(locals.seq._buffer,seqp->_buffer,locals.seq._length);
											locals.newobj=SOMDD_get_obj_from_ior(&locals.seq,ev,somddTC_SOMDServer);
										}
									}
									else
									{
										locals.newobj=SOMDObject_somDuplicateReference(objref);

										SOMOA_change_implementation(
											SOMD_SOMOAObject,
											ev,
											locals.newobj,
											locals.active_impldef);
									}
								}

								if (ev->_major)
								{
									SOMObject SOMSTAR newobj=locals.newobj;
									locals.newobj=NULL;
									if (newobj) somReleaseObjectReference(newobj);
								}

								if ((!ev->_major) && locals.newobj && !is_the_srvobj) 
								{
							/*		somPrintf("bouncing an object\n");

									SOMObject_somDumpSelf(newobj,0); */

									locals.type_id=SOMDD_get_objects_type_id(locals.newobj,ev);

									if (ev->_major)
									{
										somReleaseObjectReference(locals.newobj);
										locals.newobj=NULL;
									}
									else
									{
										if (locals.type_id)
										{
											SOMDObject__set_type_id(locals.newobj,ev,locals.type_id);
										}
									}

								}
							}

							if (!ev->_major)
							{
								somobj=locals.newobj;
								locals.newobj=NULL;
							}
						}

						if (!somobj)
						{
							RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,NO)
						}
					}
				}
			}
		}
	}

	if ((!ev->_major) && (!somobj))
	{
		if (SOMDObject_is_SOM_ref(objref,ev)) 
		{
			somobj=SOMDD_Server_parent_SOMDServer_somdSOMObjFromRef(somSelf,ev,objref);
		}
	}

	if ((!ev->_major) && (!somobj))
	{
		if (!ev->_major)
		{
			char *hint=NULL;
			unsigned long type=SOMDD_unpack_ref_data(&locals.refdata,&hint);

			switch (type)
			{
			case SOMDD_type_ImplRepository:
				if (SOMD_ImplRepObject)
				{
					somobj=ImplRepository_somDuplicateReference(SOMD_ImplRepObject);
				}
				break;
			case SOMDD_type_SOMDServer:
				somobj=SOMDD_Server_somDuplicateReference(somSelf);
				break;
			case SOMDD_type_ImplementationDef:
				if (hint)
				{
					if (SOMD_ImplRepObject)
					{
						somobj=ImplRepository_find_impldef(SOMD_ImplRepObject,ev,hint);
					}
				}
				break;
			case SOMDD_type_SOMClass:
				if (hint)
				{
					locals.clsMgr=somGetClassManagerReference();

					if (locals.clsMgr)
					{
						locals.sid=somIdFromString(hint);

						somobj=SOMClassMgr_somFindClass(locals.clsMgr,locals.sid,0,0);
					}
				}
				break;
			case SOMDD_type_SOMDServerMgr:
		#ifdef somNewObject
				somobj=somNewObject(SOMDD_ServerMgr);
		#else
				somobj=SOMDD_ServerMgrNew();
		#endif
				break;
			}
		}
	}

	RHBOPT_cleanup_pop();

	if (!somobj)
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,UnexpectedNULL,NO);
	}

	return somobj;
}

struct somddsrv_impl_up_data
{
	ImplRepositoryServer SOMSTAR irep;
#ifdef USE_THREADS
	SOMDD_MutexRef mutex;
#else
	somToken mutex;
#endif
};

RHBOPT_cleanup_begin(somddsrv_impl_up_cleanup,pv)

	struct somddsrv_impl_up_data *data=pv;
	if (data->irep) somReleaseObjectReference(data->irep);
#ifdef USE_THREADS
	SOMDD_ReleaseGlobalMutex(data->mutex);
#endif

RHBOPT_cleanup_end

SOM_Scope void SOMLINK somddsrv_impl_up(
			SOMDD_Server SOMSTAR somSelf,
			Environment *ev,
			char *impl_id,
			char *ior)
{
	struct somddsrv_impl_up_data data={NULL,NULL};

#ifdef USE_THREADS
	data.mutex=SOMDD_AcquireGlobalMutex();
#else
	data.mutex=NULL;
#endif

	RHBOPT_cleanup_push(somddsrv_impl_up_cleanup,&data);

	somPrintf("%s is up\n",impl_id);

	if (SOMD_ImplRepObject)
	{
		data.irep=ImplRepositoryServer_somDuplicateReference(SOMD_ImplRepObject);

		ImplRepositoryServer_impl_up(data.irep,ev,impl_id,ior,data.mutex);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SOMDDNotRunning,NO);
	}

	RHBOPT_cleanup_pop();
}

struct somddsrv_impl_down_data
{
	ImplRepositoryServer SOMSTAR irep;
#ifdef USE_THREADS
	SOMDD_MutexRef mutex;
#else
	somToken mutex;
#endif
};

RHBOPT_cleanup_begin(somddsrv_impl_down_cleanup,pv)

	struct somddsrv_impl_down_data *data=pv;
	if (data->irep) somReleaseObjectReference(data->irep);
#ifdef USE_THREADS
	SOMDD_ReleaseGlobalMutex(data->mutex);
#endif

RHBOPT_cleanup_end

SOM_Scope void SOMLINK somddsrv_impl_down(
			SOMDD_Server SOMSTAR somSelf,
			Environment *ev,
			char *impl_id)
{
struct somddsrv_impl_down_data data={NULL,NULL};

#ifdef USE_THREADS
	data.mutex=SOMDD_AcquireGlobalMutex();
#endif

	RHBOPT_cleanup_push(somddsrv_impl_down_cleanup,&data);

	if (SOMD_ImplRepObject)
	{
		data.irep=ImplRepositoryServer_somDuplicateReference(SOMD_ImplRepObject);

		ImplRepositoryServer_impl_down(data.irep,
			ev,impl_id,data.mutex);

		somPrintf("%s is down\n",impl_id);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,BadParm,NO);
	}

	RHBOPT_cleanup_pop();
}

SOM_Scope void SOMLINK somddsrvmgr_somInit(
	SOMDD_ServerMgr SOMSTAR somSelf)
{
	SOMDD_ServerMgr_parent_SOMDServerMgr_somInit(somSelf);
}

SOM_Scope void SOMLINK somddsrvmgr_somUninit(
	SOMDD_ServerMgr SOMSTAR somSelf)
{
	SOMDD_ServerMgr_parent_SOMDServerMgr_somUninit(somSelf);
}

SOM_Scope boolean SOMLINK somddsrvmgr_somdIsServerEnabled(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	ImplementationDef SOMSTAR impldef)
{
	somPrintf("SOMDD_ServerMgr_somdIsServerEnabled()\n");

	SOMObject_somDumpSelf(impldef,0);

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return 1;
}

SOM_Scope ORBStatus SOMLINK somddsrvmgr_somdDisableServer(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	corbastring server_alias)
{
	somPrintf("SOMDD_ServerMgr_somdDisableServer(%s)\n",server_alias);

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return SOMDERROR_NotImplemented;
}


SOM_Scope ORBStatus SOMLINK somddsrvmgr_somdEnableServer(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	corbastring server_alias)
{
	somPrintf("SOMDD_ServerMgr_somdEnableServer(%s)\n",server_alias);

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return SOMDERROR_NotImplemented;
}


SOM_Scope ORBStatus SOMLINK somddsrvmgr_somdShutdownImpl(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	ImplementationDef SOMSTAR impldef)
{
	somPrintf("SOMDD_ServerMgr_somdShutdownImpl()\n");
	SOMObject_somDumpSelf(impldef,0);

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return SOMDERROR_NotImplemented;
}

struct somddsrvmgr_somdStartServer
{
	ImplementationDef SOMSTAR impl;
	SOMObject SOMSTAR o;
	char *id;
};

RHBOPT_cleanup_begin(somddsrvmgr_somdStartServer_cleanup,pv)

	struct somddsrvmgr_somdStartServer *data=pv;

	if (data->id) SOMFree(data->id);
	if (data->impl) somReleaseObjectReference(data->impl);
	if (data->o) somReleaseObjectReference(data->o);

RHBOPT_cleanup_end

SOM_Scope ORBStatus SOMLINK somddsrvmgr_somdStartServer(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	corbastring server_alias)
{
struct somddsrvmgr_somdStartServer data={NULL,NULL,NULL};

	data.impl=ImplRepository_find_impldef_by_alias(
				SOMD_ImplRepObject,ev,server_alias);

	RHBOPT_cleanup_push(somddsrvmgr_somdStartServer_cleanup,&data);

	if (!data.impl)
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
	}
	else
	{
		data.id=ImplementationDef__get_impl_id(data.impl,ev);

		if (data.id && !ev->_major)
		{
			data.o=ImplRepositoryServer_get_active_impldef(SOMD_ImplRepObject,ev,data.id,NULL);
		}
	}

	RHBOPT_cleanup_pop();

	if (ev->_major)
	{
		return SOMDERROR_BadEnvironment;
	}

	return 0;
}

SOM_Scope ORBStatus SOMLINK somddsrvmgr_somdRestartServer(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	corbastring server_alias)
{
	somPrintf("SOMDD_ServerMgr_somdRestartServer(%s)\n",server_alias);

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return SOMDERROR_NotImplemented;
}

SOM_Scope ORBStatus SOMLINK somddsrvmgr_somdShutdownServer(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	corbastring server_alias)
{
	ImplementationDef SOMSTAR impl=ImplRepository_find_impldef_by_alias(
			SOMD_ImplRepObject,ev,server_alias);
	char *id=0;

	if (ev->_major)
	{
		return SOMDERROR_BadEnvironment;
	}

	if (!impl)
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
		return SOMDERROR_UnexpectedNULL;
	}

	id=ImplementationDef__get_impl_id(impl,ev);

	if (ev->_major)
	{
		somReleaseObjectReference(impl);
		return SOMDERROR_BadEnvironment;
	}

	somReleaseObjectReference(impl);

	if (!id)
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
		return SOMDERROR_UnexpectedNULL;
	}

	if (ImplRepositoryServer_get_stable_state(SOMD_ImplRepObject,ev,id))
	{
		ImplRepositoryServer_shutdown_server(SOMD_ImplRepObject,ev,id);
	}

	SOMFree(id);

	return 0;
}

SOM_Scope ORBStatus SOMLINK somddsrvmgr_somdListServer(
	SOMDD_ServerMgr SOMSTAR somSelf,
	Environment *ev,
	corbastring server_alias)
{
	/* this looks at the running state of the given server and
		returns 
		
		  0 if it is currently running
		  SOMDERROR_ServerDisabled - has been disabled
		  SOMDERROR_ServerNotFound - not running

		raise an exception if the server is not registered
	*/

	somPrintf("SOMDD_ServerMgr_somdListServer(%s)\n",server_alias);

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return SOMDERROR_NotImplemented;
}

struct SOMDD_get_objects_type_id
{
	Request SOMSTAR req;
	InterfaceDef SOMSTAR iface;
	SOMClass SOMSTAR cls;
	somId id;
};

RHBOPT_cleanup_begin(SOMDD_get_objects_type_id_cleanup,pv)

struct SOMDD_get_objects_type_id *data=pv;

	if (data->req) Request_somFree(data->req);
	if (data->iface) SOMObject_somFree(data->iface);
	if (data->id) SOMFree(data->id);
	if (data->cls) somReleaseClassReference(data->cls);

RHBOPT_cleanup_end

static char *SOMDD_get_objects_type_id(SOMDObject SOMSTAR dobj,Environment *ev)
{
struct SOMDD_get_objects_type_id data={NULL,NULL,NULL,NULL};
	NamedValue nv;
	char *type_id=0;

	RHBOPT_cleanup_push(SOMDD_get_objects_type_id_cleanup,&data);

	nv.argument._type=somddTC_CORBA_InterfaceDef;
	nv.argument._value=&data.iface;
	nv.name=0;
	nv.arg_modes=0;
	nv.len=0;

	SOMDObject_create_request(dobj,ev,0,"_interface",0,&nv,&data.req,0);

	if (!ev->_major)
	{
		Request_invoke(data.req,ev,0);

		{ 
			Request SOMSTAR req=data.req; 
			data.req=NULL; 
			Request_somFree(req); 
		}

		if (!ev->_major)
		{
			char *method="_get_id";
			data.cls=somNewClassReference(SOMDObject);

			if (SOMObject_somIsA(data.iface,data.cls))
			{
				nv.argument._type=TC_string;
				nv.argument._value=&type_id;
				nv.arg_modes=0;
				nv.len=0;
				nv.name=0;

				SOMDObject_create_request(data.iface,ev,0,method,0,&nv,&data.req,0);

				if (!ev->_major)
				{
					Request_invoke(data.req,ev,0);
					/* cleanup handler will delete request */
				}
			}
			else
			{
				data.id=somIdFromString(method);

				somva_SOMObject_somDispatch(
						data.iface,(somToken *)(void *)&type_id,data.id,
						data.iface,ev);
			}
		}
	}

	RHBOPT_cleanup_pop();

	return type_id;
}

void SOMDD_impl_failed(RHBProcess *proc,char *impl_id,ORBStatus rc)
{
#ifdef USE_THREADS
	SOMDD_MutexRef mutex=SOMDD_AcquireGlobalMutex();
#else
	somToken mutex=0;
#endif
	if (SOMD_ImplRepObject)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);

		ImplRepositoryServer_impl_failed(SOMD_ImplRepObject,
			&ev,impl_id,proc,rc,mutex);

		SOM_UninitEnvironment(&ev);
	}
#ifdef USE_THREADS
	SOMDD_ReleaseGlobalMutex(mutex);
#endif
}

SOM_Scope void SOMLINK somddsrv_impl_failed(
			SOMDD_Server SOMSTAR somSelf,
			Environment *ev,
			char *impl_id,
			long pid,
			ORBStatus rc)
{
	SOMDD_impl_failed(NULL,impl_id,rc);
}

struct SOMDD_get_obj_from_ior
{
	char *p;
	IOP_Codec SOMSTAR codec;
	any value;
	SOMObject SOMSTAR retVal;
};

RHBOPT_cleanup_begin(SOMDD_get_obj_from_ior_cleanup,pv)

struct SOMDD_get_obj_from_ior *data=pv;
Environment ev;

	SOM_InitEnvironment(&ev);

	if (data->p) SOMFree(data->p);
	if (data->codec) somReleaseObjectReference(data->codec);
	
	SOMD_FreeType(&ev,&data->value,TC_any);

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

SOMObject SOMSTAR SOMDD_get_obj_from_ior(_IDL_SEQUENCE_octet *seq,Environment *ev,TypeCode tcObj)
{
	SOMObject SOMSTAR RHBOPT_volatile obj=NULL;

	if (!ev->_major)
	{
		struct SOMDD_get_obj_from_ior data={NULL,NULL,{NULL,NULL},NULL};

		RHBOPT_ASSERT(TypeCode_kind(tcObj,ev)==tk_objref);

		RHBOPT_cleanup_push(SOMDD_get_obj_from_ior_cleanup,&data);

		data.codec=SOMDD_create_codec(ev);

		if (!ev->_major)
		{
			data.value=IOP_Codec_decode_value(data.codec,ev,seq,tcObj);

			if (!ev->_major)
			{
				if (tcKind(data.value._type,ev)==tk_objref)
				{
					obj=((SOMObject SOMSTAR *)data.value._value)[0];
					((SOMObject SOMSTAR *)data.value._value)[0]=NULL;
				}
			}
		}

		RHBOPT_cleanup_pop();
	}

	return obj;
}
