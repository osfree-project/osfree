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

#define SOMDServerMgr_VA_STUBS

#include <rhbopt.h>
#include <rhbsomd.h>

#ifndef SOM_Module_servmgr_Source
#define SOM_Module_servmgr_Source
#endif
#define SOMDServerMgr_Class_Source

#include "servmgr.ih"

#if !defined(SOMDServerMgr_VA_STUBS) || defined(USE_APPLE_SOM)
static boolean va_SOMDServerMgr_somddDispatch(
		SOMDServerMgr SOMSTAR somSelf,
		Environment *ev,
		somToken *retValue,
		corbastring method,
		...)
{
	boolean __result=0;
	va_list ap;
	va_start(ap,method);
	__result=SOM_Resolve(somSelf,SOMDServerMgr,somddDispatch)
			(somSelf,ev,retValue,method,ap);
	va_end(ap);
	return __result;
}
#endif

SOM_Scope ORBStatus  SOMLINK servmgr_somdShutdownServer(
		SOMDServerMgr SOMSTAR somSelf, 
		Environment *ev, 
		corbastring server_alias)
{
	ORBStatus result=SOMDERROR_UnknownError;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdShutdownServer",
			somSelf,ev,server_alias);

	return result;
}

SOM_Scope ORBStatus  SOMLINK servmgr_somdStartServer(
	SOMDServerMgr SOMSTAR somSelf, 
	Environment *ev, 
	corbastring server_alias)
{
	ORBStatus result=SOMDERROR_UnknownError;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdStartServer",
			somSelf,ev,server_alias);

	return result;
}

SOM_Scope ORBStatus  SOMLINK servmgr_somdRestartServer(
	SOMDServerMgr  SOMSTAR somSelf, 
	Environment *ev, 
	corbastring server_alias)
{
	ORBStatus result=SOMDERROR_UnknownError;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdRestartServer",
			somSelf,ev,server_alias);

	return result;
}

SOM_Scope ORBStatus  SOMLINK servmgr_somdListServer(
		SOMDServerMgr SOMSTAR somSelf, 
		Environment *ev, 
		corbastring server_alias)
{
	ORBStatus result=SOMDERROR_UnknownError;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdListServer",
			somSelf,ev,server_alias);

	return result;
}

SOM_Scope ORBStatus  SOMLINK servmgr_somdDisableServer(
		SOMDServerMgr SOMSTAR somSelf, 
		Environment *ev, 
		corbastring server_alias)
{
	ORBStatus result=SOMDERROR_UnknownError;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdDisableServer",
			somSelf,ev,server_alias);

	return result;
}

SOM_Scope ORBStatus  SOMLINK servmgr_somdEnableServer(
		SOMDServerMgr SOMSTAR somSelf, 
		Environment *ev, 
		corbastring server_alias)
{
	ORBStatus result=SOMDERROR_UnknownError;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdEnableServer",
			somSelf,ev,server_alias);

	return result;
}

SOM_Scope boolean  SOMLINK servmgr_somdIsServerEnabled(
		SOMDServerMgr SOMSTAR somSelf, 
		Environment *ev, 
		ImplementationDef SOMSTAR impldef)
{
	boolean result=0;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdIsServerEnabled",
			somSelf,ev,impldef);

	return result;
}

SOM_Scope ORBStatus SOMLINK servmgr_somdShutdownImpl(
	SOMDServerMgr SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplementationDef SOMSTAR impldef)
{
	ORBStatus result=SOMDERROR_UnknownError;

	somva_SOMDServerMgr_somddDispatch(somSelf,ev,
			(somToken *)(void *)&result,"somdShutdownImpl",
			somSelf,ev,impldef);

	return result;
}

struct servmgr__get_SOMDD_ServerMgrObject
{
	SOMDServer SOMSTAR server;
};

RHBOPT_cleanup_begin(servmgr__get_SOMDD_ServerMgrObject_cleanup,pv)

	struct servmgr__get_SOMDD_ServerMgrObject *data=pv;
	if (data->server) somReleaseObjectReference(data->server);

RHBOPT_cleanup_end

SOM_Scope SOMDServerMgr SOMSTAR SOMLINK servmgr__get_SOMDD_ServerMgrObject(
	SOMDServerMgr SOMSTAR somSelf,
	Environment *ev)
{
	SOMDServerMgr SOMSTAR RHBOPT_volatile somdd_mgr=NULL;

	if (ev->_major==NO_EXCEPTION)
	{
		RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);
		ImplId id=RHBORB_get_somdd_implid(orb);
		struct servmgr__get_SOMDD_ServerMgrObject data={NULL};

		RHBOPT_cleanup_push(servmgr__get_SOMDD_ServerMgrObject_cleanup,&data);

		data.server=SOMDObjectMgr_somdFindServer(SOMD_ObjectMgr,ev,id);

		if (data.server && !ev->_major)
		{
			somdd_mgr=SOMDServer_somdCreateObj(data.server,ev,somSelf->mtab->className,NULL);

			if (ev->_major)
			{
				somdd_mgr=NULL;
			}
		}
		else
		{
			data.server=NULL;
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);
		}

		RHBOPT_cleanup_pop();
	}

	return somdd_mgr;
}

struct servmgr_somddDispatch
{
	SOMDServerMgr SOMSTAR somdd_mgr;
	somId id;
};

RHBOPT_cleanup_begin(servmgr_somddDispatch_cleanup,pv)

	struct servmgr_somddDispatch *data=pv;

	if (data->id) SOMFree(data->id);

	if (data->somdd_mgr) somReleaseObjectReference(data->somdd_mgr);

RHBOPT_cleanup_end

SOM_Scope boolean SOMLINK servmgr_somddDispatch(
	SOMDServerMgr SOMSTAR somSelf,
	Environment *ev,
	somToken *result,
	char *method,
	va_list ap)
{
	boolean RHBOPT_volatile success=0;

	if (!ev->_major)
	{
		struct servmgr_somddDispatch data={NULL,NULL};

		RHBOPT_cleanup_push(servmgr_somddDispatch_cleanup,&data);

		data.somdd_mgr=SOMDServerMgr__get_SOMDD_ServerMgrObject(somSelf,ev);

		if (data.somdd_mgr && !ev->_major)
		{
			SOMObject SOMSTAR oldTarget;
			data.id=somIdFromString(method);

			oldTarget=somvalistGetTarget(ap);

			if (oldTarget==somSelf)
			{
				somvalistSetTarget(ap,data.somdd_mgr);
				success=SOMObject_somDispatch(data.somdd_mgr,result,data.id,ap);
			}
		}
		else
		{
			data.somdd_mgr=NULL;
		}

		RHBOPT_cleanup_pop();
	}

	if ((!success)&&(!ev->_major))
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,UnexpectedNULL,NO);
	}

	return success;
}
