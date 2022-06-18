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
#include <rhbgiops.h>

#define SOMDObjectMgr_Class_Source

#include "somdom.ih"

static SOMDServer SOMSTAR RHBSOMD_server_from_impldef(
		ImplementationDef SOMSTAR implDef,
		Environment *ev);

struct somdom_somdFindServer
{
	ImplementationDef SOMSTAR impl;
};

RHBOPT_cleanup_begin(somdom_somdFindServer_cleanup,pv)

	struct somdom_somdFindServer *data=pv;
	if (data->impl) somReleaseObjectReference(data->impl);

RHBOPT_cleanup_end

SOM_Scope SOMDServer SOMSTAR SOMLINK somdom_somdFindServer(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		ImplId serverid)
{
	SOMDServer SOMSTAR RHBOPT_volatile serv=NULL;

	if (!ev->_major)
	{
		serv=RHBORB_get_local_server(ORB__get_c_orb(SOMD_ORBObject,ev),ev,serverid);

		if ((!serv) && (!ev->_major))
		{
			struct somdom_somdFindServer data={NULL};

			RHBOPT_cleanup_push(somdom_somdFindServer_cleanup,&data);

			data.impl=ImplRepository_find_impldef(SOMD_ImplRepObject,ev,serverid);

			if (data.impl && !ev->_major)
			{
				serv=RHBSOMD_server_from_impldef(data.impl,ev);
			}

			RHBOPT_cleanup_pop();
		}
	}	

	RHBOPT_unused(somSelf)

    return serv;
}

SOM_Scope SOMDServer SOMSTAR  SOMLINK somdom_somdFindServerByName(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		corbastring servername)
{
	SOMDServer SOMSTAR RHBOPT_volatile serv=NULL;

	if (!ev->_major)
	{
		struct somdom_somdFindServer data={NULL};
		RHBOPT_cleanup_push(somdom_somdFindServer_cleanup,&data);

		data.impl=ImplRepository_find_impldef_by_alias(SOMD_ImplRepObject,ev,servername);

		if (data.impl && !ev->_major)
		{
			serv=RHBSOMD_server_from_impldef(data.impl,ev);
		}

		RHBOPT_cleanup_pop();
	}

	RHBOPT_unused(somSelf)

    return serv;
}

struct somdom_somdFindServersByClass
{
	_IDL_SEQUENCE_ImplementationDef impls;
	_IDL_SEQUENCE_SOMDServer servers;
};

RHBOPT_cleanup_begin(somdom_somdFindServersByClass_cleanup,pv)

struct somdom_somdFindServersByClass *data=pv;

	Environment ev;
	SOM_InitEnvironment(&ev);
	SOMD_FreeType(&ev,&data->impls,somdTC_sequence_SOMObject);
	SOMD_FreeType(&ev,&data->servers,somdTC_sequence_SOMObject);
	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

SOM_Scope _IDL_SEQUENCE_SOMDServer  SOMLINK somdom_somdFindServersByClass(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		Identifier objclass)
{
	_IDL_SEQUENCE_SOMDServer ret={0,0,NULL};
	struct somdom_somdFindServersByClass data={{0,0,NULL},{0,0,NULL}};

	RHBOPT_cleanup_push(somdom_somdFindServersByClass_cleanup,&data);

	if (SOMD_ImplRepObject && !ev->_major)
	{
		data.impls=ImplRepository_find_impldef_by_class(SOMD_ImplRepObject,ev,objclass);

		if (data.impls._length && !ev->_major)
		{
			unsigned int i=0;

			data.servers._maximum=data.impls._length;
			data.servers._buffer=SOMMalloc(sizeof(data.servers._buffer[0])*data.servers._maximum);

			while (i < data.impls._length)
			{
				if (data.impls._buffer[i])
				{
					SOMDServer SOMSTAR server=RHBSOMD_server_from_impldef(data.impls._buffer[i],ev);

					if (ev->_major)
					{
						somExceptionFree(ev);
					}
					else
					{
						data.servers._buffer[data.servers._length++]=server;
					}
				}

				i++;
			}
		}
	}

	if (!ev->_major)
	{
		ret=data.servers;
		data.servers._buffer=NULL;
		data.servers._length=0;
		data.servers._maximum=0;
	}

	RHBOPT_cleanup_pop();

    return ret;
}

SOM_Scope SOMDServer SOMSTAR  SOMLINK somdom_somdFindAnyServerByClass(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		Identifier objclass)
{
	SOMDServer SOMSTAR RHBOPT_volatile server=NULL;
	struct somdom_somdFindServersByClass data={{0,0,NULL},{0,0,NULL}};

	RHBOPT_cleanup_push(somdom_somdFindServersByClass_cleanup,&data);

	if (SOMD_ImplRepObject && !ev->_major)
	{
		data.impls=ImplRepository_find_impldef_by_class(SOMD_ImplRepObject,ev,objclass);

		if (data.impls._length && !ev->_major)
		{
			unsigned int i=0;

			while (i < data.impls._length)
			{
				if (data.impls._buffer[i])
				{
					server=RHBSOMD_server_from_impldef(data.impls._buffer[i],ev);

					if (ev->_major)
					{
						somExceptionFree(ev);
						server=NULL;
					}
					else
					{
						break;
					}
				}

				i++;
			}
		}
	}

	RHBOPT_cleanup_pop();

	if ((!server) && (!ev->_major))
	{
		RHBOPT_throw_StExcep(ev,UNKNOWN,UnexpectedNULL,MAYBE);
	}

    return server;
}

#ifdef SOMDObjectMgr_somDefaultInit
SOM_Scope void SOMLINK somdom_somDefaultInit(
	SOMDObjectMgr SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SOMDObjectMgrData *somThis;

	SOMDObjectMgr_BeginInitializer_somDefaultInit
	SOMDObjectMgr_Init_ObjectMgr_somDefaultInit(somSelf,ctrl);
	RHBOPT_unused(somThis);
}
#else
SOM_Scope void  SOMLINK somdom_somInit(SOMDObjectMgr SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
    SOMDObjectMgr_parent_ObjectMgr_somInit(somSelf);
}
#endif

struct somdom_somdNewObject
{
	SOMObject SOMSTAR result;
	_IDL_SEQUENCE_SOMDServer servers;
};

RHBOPT_cleanup_begin(somdom_somdNewObject_cleanup,pv)

struct somdom_somdNewObject *data=pv;

	if (data->servers._buffer)
	{
		unsigned long i=data->servers._length;
		while (i--)
		{
			if (data->servers._buffer[i])
			{
				somReleaseObjectReference(data->servers._buffer[i]);
			}
		}
		SOMFree(data->servers._buffer);
	}

RHBOPT_cleanup_end

SOM_Scope SOMObject SOMSTAR SOMLINK somdom_somdNewObject(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		Identifier objclass, 
		corbastring hints)
{
struct somdom_somdNewObject data={NULL,{0,0,NULL}};

	RHBOPT_cleanup_push(somdom_somdNewObject_cleanup,&data);

	data.servers=SOMDObjectMgr_somdFindServersByClass(somSelf,ev,objclass);

	if (data.servers._length && !ev->_major)
	{
		unsigned long i=0;

		while (i < data.servers._length)
		{
			if (data.servers._buffer[i])
			{
				somdExceptionFree(ev);

				data.result=SOMDServer_somdCreateObj(data.servers._buffer[i],ev,objclass,hints);

				if (ev->_major || !data.result)
				{
					data.result=NULL;
				}
				else
				{
					break;
				}
			}

			i++;
		}
	}

	RHBOPT_cleanup_pop();

	return data.result;
}

SOM_Scope corbastring  SOMLINK somdom_somdGetIdFromObject(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR obj)
{
    return (SOMDObjectMgr_parent_ObjectMgr_somdGetIdFromObject(somSelf, 
                                                               ev, 
                                                               obj));
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somdom_somdGetObjectFromId(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		corbastring id)
{
    return (SOMDObjectMgr_parent_ObjectMgr_somdGetObjectFromId(somSelf, 
                                                               ev, 
                                                               id));
}

SOM_Scope void  SOMLINK somdom_somdReleaseObject(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR obj)
{
	if (obj)
	{
#ifdef SOMObject_somRelease
		SOMObject_somRelease(obj);
#else
		if (SOMObject_somIsA(obj,_SOMRefObject))
		{
			SOMRefObject_somRelease(obj);
		}
#endif
	}
}

struct somdom_somdDestroyObject
{
	ImplementationDef SOMSTAR implDef;
	SOMDServer SOMSTAR server;
	SOMObject SOMSTAR obj;
};

RHBOPT_cleanup_begin(somdom_somdDestroyObject_cleanup,pv)

struct somdom_somdDestroyObject *data=pv;

	if (data->obj)
	{
		SOMObject_somFree(data->obj);
	}

	if (data->implDef)
	{
		somReleaseObjectReference(data->implDef);
	}
	if (data->server)
	{
		somReleaseObjectReference(data->server);
	}

RHBOPT_cleanup_end

SOM_Scope void  SOMLINK somdom_somdDestroyObject(
		SOMDObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR obj)
{
struct somdom_somdDestroyObject data={NULL,NULL,NULL};

	data.obj=obj;

	RHBOPT_cleanup_push(somdom_somdDestroyObject_cleanup,&data);

	if (obj && !ev->_major)
	{
		data.implDef=SOMDObject_get_implementation(obj,ev);

		if (data.implDef && !ev->_major)
		{
			data.server=RHBSOMD_server_from_impldef(data.implDef,ev);

			if (data.server)
			{
				SOMDServer_somdDeleteObj(data.server,ev,obj);
			}
		}
	}

	RHBOPT_cleanup_pop();
}

#ifdef Context_somDestruct
SOM_Scope void SOMLINK somdom_somDestruct(
	SOMDObjectMgr SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	Environment ev;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SOMDObjectMgrData *somThis;

	SOMDObjectMgr_BeginDestructor
#else
SOM_Scope void SOMLINK somdom_somUninit(
		SOMDObjectMgr SOMSTAR somSelf)
{
	Environment ev;
#endif

	RHBOPT_unused(somThis);

	SOM_InitEnvironment(&ev);
	if (somSelf==SOMD_ObjectMgr)
	{
		SOMD_ObjectMgr=NULL;
	}

	if (SOMMSingleInstanceClassData.sommSingleInstanceFreed)
	{
		SOMMSingleInstance_sommSingleInstanceFreed(somSelf->mtab->classObject,&ev,somSelf);
	}

	SOM_UninitEnvironment(&ev);

#ifdef Context_somDestruct
	SOMDObjectMgr_EndDestructor
#else
	SOMDObjectMgr_parent_ObjectMgr_somUninit(somSelf);
#endif
}

struct RHBSOMD_server_from_impldef
{
	char *id;
	char *impl_class;
/*	TypeCode tc2,tc; */
	RHBContained *cnd;
	RHBInterfaceDef *iface;
	RHBImplementationDef *impldef;
/*	somId sid;*/
	any d;
	IIOP_ProfileBody_1_0 body_1_0;
};

RHBOPT_cleanup_begin(RHBSOMD_server_from_impldef_cleanup,pv)

	struct RHBSOMD_server_from_impldef *data=pv;

	Environment ev;
	SOM_InitEnvironment(&ev);

	SOMD_FreeType(&ev,&data->id,TC_string);
/*	SOMD_FreeType(&ev,&data->tc2,TC_TypeCode); */
/*	SOMD_FreeType(&ev,&data->tc,TC_TypeCode); */
	SOMD_FreeType(&ev,&data->d,TC_any);
	SOMD_FreeType(&ev,&data->impl_class,TC_string);
	SOMD_FreeType(&ev,&data->body_1_0,somdTC_IIOP_ProfileBody_1_0);

	if (data->cnd) RHBContained_Release(data->cnd);
	if (data->iface) RHBInterfaceDef_Release(data->iface);
	if (data->impldef) RHBImplementationDef_Release(data->impldef);
/*	if (data->sid) SOMFree(data->sid); */

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

static SOMDServer SOMSTAR RHBSOMD_server_from_impldef(ImplementationDef SOMSTAR impl,Environment *ev)
{
	/* this is a bit of a monster!!! */

	SOMDServer SOMSTAR RHBOPT_volatile serv=NULL;

	if (impl && !ev->_major) 
	{
		struct RHBSOMD_server_from_impldef data={
				NULL,
				NULL,
			/*	NULL, */
			/*	NULL, */
				NULL,
				NULL,
				NULL,
			/*	NULL, */
				{NULL,NULL},
				{{1,0},NULL,0,{0,0,NULL}}
		};

		RHBOPT_cleanup_push(RHBSOMD_server_from_impldef_cleanup,&data);

		data.id=ImplementationDef__get_impl_id(impl,ev);

		if (data.id && !ev->_major)
		{
			RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

			if (!ev->_major)
			{
				serv=RHBORB_get_local_server(orb,ev,data.id);

				if (!serv)
				{
					ImplementationDef_octet_seqP svr_objref=NULL;

					somdExceptionFree(ev);

					svr_objref=ImplementationDef__get_svr_objref(impl,ev);

					if (svr_objref && !ev->_major)
					{
				/*		data.tc2=TypeCodeNew(tk_objref,"::SOMDServer"); */

						data.d=IOP_Codec_decode_value(orb->iop_codec,ev,svr_objref,
							somdTC_SOMDServer); /* data.tc2 */

						if (!ev->_major)
						{
							if (data.d._value)
							{
								SOMObject SOMSTAR *h=data.d._value;
								serv=*h;
								*h=NULL;
							}
						}
					}
				}
			}

			if ((!serv) && (!ev->_major))
			{
				data.impl_class=ImplementationDef__get_impl_server_class(impl,ev);

				if ((!data.impl_class) && !ev->_major)
				{
					data.impl_class=somd_dupl_string("::SOMDServer");
				}
				else
				{
					if (!ev->_major)
					{
						if (memcmp(data.impl_class,"::",2))
						{
							size_t len=strlen(data.impl_class);
							char *buf=SOMMalloc(len+3);
							buf[0]=':';
							buf[1]=':';
							memcpy(buf+2,data.impl_class,len+1);
							SOMFree(data.impl_class);
							data.impl_class=buf;
						}
					}

				}

				if (!ev->_major)
				{
					data.cnd=RHBORB_get_contained(orb,ev,data.impl_class);

					if (data.cnd && !ev->_major)
					{
						data.iface=RHBORB_get_interface(orb,ev,
								RHBContained_get_id(data.cnd,ev));
					}

					data.body_1_0.object_key=somdGetDefaultObjectKey(ev,data.id);

					/* we want at least a SOMDServer */

					/* data.tc=TypeCodeNew(tk_objref,"::SOMDServer"); */

					if (RHBORB_isSOMDObject(orb,impl))
					{
						RHBObject *obj=SOMDObject__get_c_proxy(impl,ev);
						/* it's a remote impl */
						if (obj->impl)
						{
							data.body_1_0.host=somd_dupl_string(obj->impl->address.host);
							data.body_1_0.port=obj->impl->address.port;
						}
					}

					if (!data.body_1_0.host)
					{
		/*				data.sid=somIdFromString("_get_impl_hostname"); */

						if (ev->_major)
						{
							debug_somPrintf(("pre-dispatch error %s\n",somExceptionId(ev)));
							somExceptionFree(ev);
						}

/*			body.host=ImplementationDef__get_impl_hostname(impl,ev);*/

						somva_SOMObject_somDispatch(
								impl,(somToken *)(void *)&data.body_1_0.host,orb->somId_get_impl_hostname,
								impl,ev); /* data.sid */

						if (ev->_major)
						{
							debug_somPrintf(("::somDispatch(_get_impl_hostname) returned %s\n",somExceptionId(ev)));
							somExceptionFree(ev);
						}
					}

					if (!data.body_1_0.port)
					{
						data.body_1_0.port=RHBORB_get_SOMDPORT(orb);
					}
	
					somExceptionFree(ev);

					if (!data.body_1_0.host)
					{
						char *localhost=RHBORB_get_HOSTNAME(orb);

						data.body_1_0.host=somd_dupl_string(localhost);
					}

					data.impldef=RHBORB_get_impl(orb,ev,&data.body_1_0);

					if (data.impldef)
					{
						serv=RHBORB_get_object(orb,
							ev,
							0,
							data.impldef,
							data.iface,
							&data.body_1_0.object_key,
							somdTC_SOMDServer); /* data.tc */
					}
					else
					{
						serv=NULL;
						RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
					}
				}
			}
		}

		RHBOPT_cleanup_pop();
	}

	if (!serv)
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,ServerNotFound,NO);
	}

	return serv;
}

