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

#define PortableServer_DynamicImpl_Class_Source

/*
#define PortableServer_POA_Class_Source
#define PortableServer_POAManager_Class_Source
#define PortableServer_Current_Class_Source
#define PortableServer_ThreadPolicy_Class_Source
#define PortableServer_LifespanPolicy_Class_Source
#define PortableServer_IdUniquenessPolicy_Class_Source
#define PortableServer_IdAssignmentPolicy_Class_Source
#define PortableServer_ImplicitActivationPolicy_Class_Source
#define PortableServer_ServantRetentionPolicy_Class_Source
#define PortableServer_RequestProcessingPolicy_Class_Source
#define PortableServer_ServantManager_Class_Source
#define PortableServer_ServantActivator_Class_Source
#define PortableServer_ServantLocator_Class_Source
#define PortableServer_AdapterActivator_Class_Source
*/


#include <poa.ih>

/*******************************************************
 *
 * PortableServer_POA_Class_Source
 *
 */

#ifdef PortableServer_POA_Class_Source
/* overridden methods for ::PortableServer::POA */
/* introduced methods for ::PortableServer::POA */
/* introduced method ::PortableServer::POA::create_POA */
SOM_Scope PortableServer_POA SOMSTAR SOMLINK poa_create_POA(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring adapter_name,
	/* in */ PortableServer_POAManager SOMSTAR a_POAManager,
	/* in */ CORBA_PolicyList *policies)
{
	PortableServer_POA SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::find_POA */
SOM_Scope PortableServer_POA SOMSTAR SOMLINK poa_find_POA(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring adapter_name,
	/* in */ boolean activate_it)
{
	PortableServer_POA SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::destroy */
SOM_Scope void SOMLINK poa_destroy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean etherealize_objects,
	/* in */ boolean wait_for_completion)
{
}
/* introduced method ::PortableServer::POA::create_thread_policy */
SOM_Scope PortableServer_ThreadPolicy SOMSTAR SOMLINK poa_create_thread_policy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ThreadPolicyValue value)
{
	PortableServer_ThreadPolicy SOMSTAR __result=NULL;
	if (!ev->_major)
	{
		__result=PortableServer_ThreadPolicyNew();
		if (__result)
		{
			PortableServer_ThreadPolicy__set_value(__result,ev,value);
			if (ev->_major)
			{
				PortableServer_ThreadPolicy_somRelease(__result);
				__result=NULL;
			}
		}
	}
	return __result;
}
/* introduced method ::PortableServer::POA::create_lifespan_policy */
SOM_Scope PortableServer_LifespanPolicy SOMSTAR SOMLINK poa_create_lifespan_policy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_LifespanPolicyValue value)
{
	PortableServer_LifespanPolicy SOMSTAR __result=NULL;
	if (!ev->_major)
	{
		__result=PortableServer_LifespanPolicyNew();
		if (__result)
		{
			PortableServer_LifespanPolicy__set_value(__result,ev,value);
			if (ev->_major)
			{
				PortableServer_LifespanPolicy_somRelease(__result);
				__result=NULL;
			}
		}
	}
	return __result;
}
/* introduced method ::PortableServer::POA::create_id_uniqueness_policy */
SOM_Scope PortableServer_IdUniquenessPolicy SOMSTAR SOMLINK poa_create_id_uniqueness_policy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_IdUniquenessPolicyValue value)
{
	PortableServer_IdUniquenessPolicy SOMSTAR __result=NULL;
	if (!ev->_major)
	{
		__result=PortableServer_IdUniquenessPolicyNew();
		if (__result)
		{
			PortableServer_IdUniquenessPolicy__set_value(__result,ev,value);
			if (ev->_major)
			{
				PortableServer_IdUniquenessPolicy_somRelease(__result);
				__result=NULL;
			}
		}
	}
	return __result;
}
/* introduced method ::PortableServer::POA::create_id_assignment_policy */
SOM_Scope PortableServer_IdAssignmentPolicy SOMSTAR SOMLINK poa_create_id_assignment_policy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_IdAssignmentPolicyValue value)
{
	PortableServer_IdAssignmentPolicy SOMSTAR __result=NULL;
	if (!ev->_major)
	{
		__result=PortableServer_IdAssignmentPolicyNew();
		if (__result)
		{
			PortableServer_IdAssignmentPolicy__set_value(__result,ev,value);
			if (ev->_major)
			{
				PortableServer_IdAssignmentPolicy_somRelease(__result);
				__result=NULL;
			}
		}
	}
	return __result;
}
/* introduced method ::PortableServer::POA::create_implicit_activation_policy */
SOM_Scope PortableServer_ImplicitActivationPolicy SOMSTAR SOMLINK poa_create_implicit_activation_policy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ImplicitActivationPolicyValue value)
{
	PortableServer_ImplicitActivationPolicy SOMSTAR __result=NULL;
	if (!ev->_major)
	{
		__result=PortableServer_ImplicitActivationPolicyNew();
		if (__result)
		{
			PortableServer_ImplicitActivationPolicy__set_value(__result,ev,value);
			if (ev->_major)
			{
				PortableServer_ImplicitActivationPolicy_somRelease(__result);
				__result=NULL;
			}
		}
	}
	return __result;
}
/* introduced method ::PortableServer::POA::create_servant_retention_policy */
SOM_Scope PortableServer_ServantRetentionPolicy SOMSTAR SOMLINK poa_create_servant_retention_policy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ServantRetentionPolicyValue value)
{
	PortableServer_ServantRetentionPolicy SOMSTAR __result=NULL;
	if (!ev->_major)
	{
		__result=PortableServer_ServantRetentionPolicyNew();
		if (__result)
		{
			PortableServer_ServantRetentionPolicy__set_value(__result,ev,value);
			if (ev->_major)
			{
				PortableServer_ServantRetentionPolicy_somRelease(__result);
				__result=NULL;
			}
		}
	}
	return __result;
}
/* introduced method ::PortableServer::POA::create_request_processing_policy */
SOM_Scope PortableServer_RequestProcessingPolicy SOMSTAR SOMLINK poa_create_request_processing_policy(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_RequestProcessingPolicyValue value)
{
	PortableServer_RequestProcessingPolicy SOMSTAR __result=NULL;
	if (!ev->_major)
	{
		__result=PortableServer_RequestProcessingPolicyNew();
		if (__result)
		{
			PortableServer_RequestProcessingPolicy__set_value(__result,ev,value);
			if (ev->_major)
			{
				PortableServer_RequestProcessingPolicy_somRelease(__result);
				__result=NULL;
			}
		}
	}
	return __result;
}
/* introduced method ::PortableServer::POA::_get_the_name */
SOM_Scope corbastring SOMLINK poa__get_the_name(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev)
{
	corbastring __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::_get_the_parent */
SOM_Scope PortableServer_POA SOMSTAR SOMLINK poa__get_the_parent(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_POA SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::_get_the_children */
SOM_Scope PortableServer_POAList SOMLINK poa__get_the_children(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_POAList __result={0,0,NULL};
	return __result;
}
/* introduced method ::PortableServer::POA::_get_the_POAManager */
SOM_Scope PortableServer_POAManager SOMSTAR SOMLINK poa__get_the_POAManager(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_POAManager SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::_set_the_activator */
SOM_Scope void SOMLINK poa__set_the_activator(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_AdapterActivator SOMSTAR the_activator)
{
}
/* introduced method ::PortableServer::POA::_get_the_activator */
SOM_Scope PortableServer_AdapterActivator SOMSTAR SOMLINK poa__get_the_activator(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_AdapterActivator SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::get_servant_manager */
SOM_Scope PortableServer_ServantManager SOMSTAR SOMLINK poa_get_servant_manager(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_ServantManager SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::set_servant_manager */
SOM_Scope void SOMLINK poa_set_servant_manager(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ServantManager SOMSTAR imgr)
{
}
/* introduced method ::PortableServer::POA::get_servant */
SOM_Scope PortableServer_Servant SOMLINK poa_get_servant(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_Servant __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::set_servant */
SOM_Scope void SOMLINK poa_set_servant(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_Servant p_servant)
{
}
/* introduced method ::PortableServer::POA::activate_object */
SOM_Scope PortableServer_ObjectId SOMLINK poa_activate_object(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_Servant p_servant)
{
	PortableServer_ObjectId __result={0,0,NULL};
	return __result;
}
/* introduced method ::PortableServer::POA::activate_object_with_id */
SOM_Scope void SOMLINK poa_activate_object_with_id(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *id,
	/* in */ PortableServer_Servant p_servant)
{
}
/* introduced method ::PortableServer::POA::deactivate_object */
SOM_Scope void SOMLINK poa_deactivate_object(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid)
{
}
/* introduced method ::PortableServer::POA::create_reference */
SOM_Scope SOMDObject SOMSTAR SOMLINK poa_create_reference(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ RepositoryId intf)
{
	SOMDObject SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::create_reference_with_id */
SOM_Scope SOMDObject SOMSTAR SOMLINK poa_create_reference_with_id(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid,
	/* in */ RepositoryId intf)
{
	SOMDObject SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::servant_to_id */
SOM_Scope PortableServer_ObjectId SOMLINK poa_servant_to_id(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_Servant p_servant)
{
	PortableServer_ObjectId __result={0,0,NULL};
	return __result;
}
/* introduced method ::PortableServer::POA::servant_to_reference */
SOM_Scope SOMDObject SOMSTAR SOMLINK poa_servant_to_reference(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_Servant p_servant)
{
	SOMDObject SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::reference_to_servant */
SOM_Scope PortableServer_Servant SOMLINK poa_reference_to_servant(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMDObject SOMSTAR reference)
{
	PortableServer_Servant __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::reference_to_id */
SOM_Scope PortableServer_ObjectId SOMLINK poa_reference_to_id(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMDObject SOMSTAR reference)
{
	PortableServer_ObjectId __result={0,0,NULL};
	return __result;
}
/* introduced method ::PortableServer::POA::id_to_servant */
SOM_Scope PortableServer_Servant SOMLINK poa_id_to_servant(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid)
{
	PortableServer_Servant __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::POA::id_to_reference */
SOM_Scope SOMDObject SOMSTAR SOMLINK poa_id_to_reference(
	PortableServer_POA SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid)
{
	SOMDObject SOMSTAR __result=NULL;
	return __result;
}

#endif /* PortableServer_POA_Class_Source */

/*******************************************************
 *
 * PortableServer_Current_Class_Source
 *
 */

#ifdef PortableServer_Current_Class_Source
/* overridden methods for ::PortableServer::Current */
/* introduced methods for ::PortableServer::Current */
/* introduced method ::PortableServer::Current::get_POA */
SOM_Scope PortableServer_POA SOMSTAR SOMLINK pscrrnt_get_POA(
	PortableServer_Current SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_POA SOMSTAR __result=NULL;
	return __result;
}
/* introduced method ::PortableServer::Current::get_object_id */
SOM_Scope PortableServer_ObjectId SOMLINK pscrrnt_get_object_id(
	PortableServer_Current SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_ObjectId __result={0,0,NULL};
	return __result;
}

SOM_Scope PortableServer_ServantRetentionPolicyValue SOMLINK ps_srp__get_value(
	PortableServer_ServantRetentionPolicy SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_ServantRetentionPolicyData *somThis=PortableServer_ServantRetentionPolicyGetData(somSelf);
	return somThis->value;
}

SOM_Scope void SOMLINK ps_srp__set_value(
	PortableServer_ServantRetentionPolicy SOMSTAR somSelf,
	Environment *ev,
	PortableServer_ServantRetentionPolicyValue value)
{
	PortableServer_ServantRetentionPolicyData *somThis=PortableServer_ServantRetentionPolicyGetData(somSelf);
	somThis->value=value;
}

SOM_Scope CORBA_PolicyType SOMLINK ps_srp__get_policy_type(
	PortableServer_ServantRetentionPolicy SOMSTAR somSelf,
	Environment *ev)
{
	return PortableServer_SERVANT_RETENTION_POLICY_ID;
}

SOM_Scope PortableServer_LifespanPolicyValue SOMLINK ps_lsp__get_value(
	PortableServer_LifespanPolicy SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_LifespanPolicyData *somThis=PortableServer_LifespanPolicyGetData(somSelf);
	return somThis->value;
}

SOM_Scope void SOMLINK ps_lsp__set_value(
	PortableServer_LifespanPolicy SOMSTAR somSelf,
	Environment *ev,
	PortableServer_LifespanPolicyValue value)
{
	PortableServer_LifespanPolicyData *somThis=PortableServer_LifespanPolicyGetData(somSelf);
	somThis->value=value;
}

SOM_Scope CORBA_PolicyType SOMLINK ps_lsp__get_policy_type(
	PortableServer_LifespanPolicy SOMSTAR somSelf,
	Environment *ev)
{
	return PortableServer_LIFESPAN_POLICY_ID;
}

SOM_Scope PortableServer_ThreadPolicyValue SOMLINK ps_tp__get_value(
	PortableServer_ThreadPolicy SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_ThreadPolicyData *somThis=PortableServer_ThreadPolicyGetData(somSelf);
	return somThis->value;
}

SOM_Scope void SOMLINK ps_tp__set_value(
	PortableServer_ThreadPolicy SOMSTAR somSelf,
	Environment *ev,
	PortableServer_ThreadPolicyValue value)
{
	PortableServer_ThreadPolicyData *somThis=PortableServer_ThreadPolicyGetData(somSelf);
	somThis->value=value;
}

SOM_Scope CORBA_PolicyType SOMLINK ps_tp__get_policy_type(
	PortableServer_ThreadPolicy SOMSTAR somSelf,
	Environment *ev)
{
	return PortableServer_THREAD_POLICY_ID;
}

SOM_Scope PortableServer_RequestProcessingPolicyValue SOMLINK ps_rrp__get_value(
	PortableServer_RequestProcessingPolicy SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_RequestProcessingPolicyData *somThis=PortableServer_RequestProcessingPolicyGetData(somSelf);
	return somThis->value;
}

SOM_Scope void SOMLINK ps_rrp__set_value(
	PortableServer_RequestProcessingPolicy SOMSTAR somSelf,
	Environment *ev,
	PortableServer_RequestProcessingPolicyValue value)
{
	PortableServer_RequestProcessingPolicyData *somThis=PortableServer_RequestProcessingPolicyGetData(somSelf);
	somThis->value=value;
}

SOM_Scope CORBA_PolicyType SOMLINK ps_rrp__get_policy_type(
	PortableServer_RequestProcessingPolicy SOMSTAR somSelf,
	Environment *ev)
{
	return PortableServer_REQUEST_PROCESSING_POLICY_ID;
}

SOM_Scope PortableServer_ImplicitActivationPolicyValue SOMLINK ps_impap__get_value(
	PortableServer_ImplicitActivationPolicy SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_ImplicitActivationPolicyData *somThis=PortableServer_ImplicitActivationPolicyGetData(somSelf);

	return somThis->value;
}

SOM_Scope void SOMLINK ps_impap__set_value(
	PortableServer_ImplicitActivationPolicy SOMSTAR somSelf,
	Environment *ev,
	PortableServer_ImplicitActivationPolicyValue value)
{
	PortableServer_ImplicitActivationPolicyData *somThis=PortableServer_ImplicitActivationPolicyGetData(somSelf);
	somThis->value=value;
}

SOM_Scope CORBA_PolicyType SOMLINK ps_impap__get_policy_type(
	PortableServer_ImplicitActivationPolicy SOMSTAR somSelf,
	Environment *ev)
{
	return PortableServer_IMPLICIT_ACTIVATION_POLICY_ID;
}

SOM_Scope PortableServer_IdUniquenessPolicyValue SOMLINK ps_iup__get_value(
	PortableServer_IdUniquenessPolicy SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_IdUniquenessPolicyData *somThis=PortableServer_IdUniquenessPolicyGetData(somSelf);
	return somThis->value;
}

SOM_Scope CORBA_PolicyType SOMLINK ps_iup__get_policy_type(
	PortableServer_IdUniquenessPolicy SOMSTAR somSelf,
	Environment *ev)
{
	return PortableServer_ID_UNIQUENESS_POLICY_ID;
}

SOM_Scope void SOMLINK ps_iup__set_value(
	PortableServer_IdUniquenessPolicy SOMSTAR somSelf,
	Environment *ev,
	PortableServer_IdUniquenessPolicyValue value)
{
	PortableServer_IdUniquenessPolicyData *somThis=PortableServer_IdUniquenessPolicyGetData(somSelf);
	somThis->value=value;
}

SOM_Scope PortableServer_IdAssignmentPolicyValue SOMLINK ps_idap__get_value(
	PortableServer_IdAssignmentPolicy SOMSTAR somSelf,
	Environment *ev)
{
	PortableServer_IdAssignmentPolicyData *somThis=PortableServer_IdAssignmentPolicyGetData(somSelf);
	return somThis->value;
}

SOM_Scope CORBA_PolicyType SOMLINK ps_idap__get_policy_type(
	PortableServer_IdAssignmentPolicy SOMSTAR somSelf,
	Environment *ev)
{
	return PortableServer_ID_ASSIGNMENT_POLICY_ID;
}

SOM_Scope void SOMLINK ps_idap__set_value(
	PortableServer_IdAssignmentPolicy SOMSTAR somSelf,
	Environment *ev,
	PortableServer_IdAssignmentPolicyValue value)
{
	PortableServer_IdAssignmentPolicyData *somThis=PortableServer_IdAssignmentPolicyGetData(somSelf);
	somThis->value=value;
}

SOM_Scope void SOMLINK ps_sa_etherealize(
	PortableServer_ServantActivator SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid,
	/* in */ PortableServer_POA SOMSTAR adapter,
	/* in */ PortableServer_Servant serv,
	/* in */ boolean cleanup_in_progress,
	/* in */ boolean remaining_activations)
{
}

SOM_Scope PortableServer_Servant SOMLINK ps_sa_incarnate(
	PortableServer_ServantActivator SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid,
	/* in */ PortableServer_POA SOMSTAR adapter)
{
	return NULL;
}

SOM_Scope void SOMLINK ps_poam_activate(
	PortableServer_POAManager SOMSTAR somSelf,
	Environment *ev)
{
}

SOM_Scope void SOMLINK ps_poam_discard_requests(
	PortableServer_POAManager SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean wait_for_completion)
{
}

SOM_Scope void SOMLINK ps_poam_deactivate(
	PortableServer_POAManager SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean etherealize_objects,
	/* in */ boolean wait_for_completion)
{
}

SOM_Scope void SOMLINK ps_poam_hold_requests(
	PortableServer_POAManager SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean wait_for_completion)
{
}

SOM_Scope PortableServer_POAManager_State SOMLINK ps_poam_get_state(
	PortableServer_POAManager SOMSTAR somSelf,
	Environment *ev)
{
	return 0;
}
#endif

#ifdef PortableServer_AdapterActivator_Class_Source
SOM_Scope boolean SOMLINK ps_aa_unknown_adapter(
	PortableServer_AdapterActivator SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_POA SOMSTAR parent,
	/* in */ corbastring name)
{
	return 0;
}

SOM_Scope void SOMLINK ps_sl_postinvoke(
	PortableServer_ServantLocator SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid,
	/* in */ PortableServer_POA SOMSTAR adapter,
	/* in */ Identifier operation,
	/* in */ PortableServer_ServantLocator_Cookie the_cookie,
	/* in */ PortableServer_Servant the_servant)
{
}

SOM_Scope PortableServer_Servant SOMLINK ps_sl_preinvoke(
	PortableServer_ServantLocator SOMSTAR somSelf,
	Environment *ev,
	/* in */ PortableServer_ObjectId *oid,
	/* in */ PortableServer_POA SOMSTAR adapter,
	/* in */ Identifier operation,
	/* out */ PortableServer_ServantLocator_Cookie *the_cookie)
{
	return NULL;
}


#endif /* PortableServer_AdapterActivator_Class_Source */


SOM_Scope void SOMLINK dynimpl_invoke(
		PortableServer_DynamicImpl SOMSTAR somSelf,
		Environment *ev,
		ServerRequest SOMSTAR request)
{
	any a={NULL,NULL};
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	ServerRequest_set_result(request,ev,&a);
	somdExceptionFree(ev);
}

SOM_Scope RepositoryId SOMLINK dynimpl_primary_interface(
		PortableServer_DynamicImpl SOMSTAR somSelf,
		Environment *ev,
		PortableServer_ObjectId *object_id,
		PortableServer_POA SOMSTAR poa)
{
	RepositoryId id=NULL;
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	return id;
}

