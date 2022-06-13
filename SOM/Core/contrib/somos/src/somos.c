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

#define somOS_Server_Class_Source
#define somOS_ServiceBase_Class_Source
#define somOS_ServiceBasePRef_Class_Source
#define somOS_ServiceBaseCORBA_Class_Source
#define somOS_ServiceBasePRefCORBA_Class_Source

#include <rhbsomos.h>
#include <somos.ih>

SOM_Scope SOMObject SOMSTAR SOMLINK somos_servbase_corba_pref_init_for_object_reactivation(
	somOS_ServiceBasePRefCORBA SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_somdserv_make_persistent_ref(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR referenced_object)
{
    /* - Make an entry in the reference-data-table and persistent storage.
         An empty metadata (place holder) is stored in the persistent storage.
       - If an entry already exists then no action is taken and the operation
         is considered successful but the exception
         SysAdminException::ExExists is returned (see RETURN value below).
       - RETURNS OBJECT_NIL              if unsuccessful
                 referenced_object       if successful
	 */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(referenced_object)

	return 0;
}

SOM_Scope void SOMLINK somos_somdserv_delete_ref(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR referenced_object)
{
      /* Remove its entry in the persistent storage.
         It does not delete the in-memory object or the entry in reference-data-table.
	  */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(referenced_object)
}

SOM_Scope boolean SOMLINK somos_somdserv_has_persistent_ref(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR referenced_object)
{
    /* Indicate whether the specified object has a persistent reference.
       RETURNS TRUE iff an entry exists in reference-data-table or in the
                persistent storage, or it is a transaction ref.
	*/
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(referenced_object)

	return 0;
}

SOM_Scope void SOMLINK somos_somdserv_passivate_object(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR referenced_object)
{
	/* Capture and store the metastate,
       uninit_for_object_passivation (removes entries from reference table),
       and somdestruct 
       It DOES NOT delete references on persistent storage */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(referenced_object)
}

SOM_Scope void SOMLINK somos_somdserv_passivate_all_objects(
	somOS_Server SOMSTAR somSelf,
	Environment *ev)
{
     /* passivate all active objects */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	somPrintf("%s::passivate_all_objects\n",somOS_Server_somGetClassName(somSelf));
}

SOM_Scope void SOMLINK somos_somdserv_store_service_metastate(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR referenced_object,
	/* in */ somOS_service_id_e somos_service_id,
	/* in */ any *service_metadata)
{
    /* Store the metastate (service_metadata) of a service (somos_service_id)
       for the specified object (referenced_object).*/
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(referenced_object)
	RHBOPT_unused(somos_service_id)
	RHBOPT_unused(service_metadata)
}

SOM_Scope void SOMLINK somos_somdserv_store_metastate(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR referenced_object)
{
     /* Call capture on the specified object and put it in persistent storage.
        Stores metadata of all service */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(referenced_object)
}

SOM_Scope void SOMLINK somos_somdserv_restore_metastate(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR referenced_object)
{
     /* Get metadata from persistent storage and call reinit() on
        referenced_object */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(referenced_object)
}

SOM_Scope SOMDObject SOMSTAR SOMLINK somos_somdserv_somdRefFromSOMObj(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR somobj)
{
	SOMClass SOMSTAR cls;
	boolean b;

	/* If it is a SOMDObject or it is  not a Servicebase object or
       it is a transient ServiceBase object which is not a transaction
          object THEN let the SOMDServer handle it.
       For a persistent ServiceBase object (i.e., which has an entry
          in reference-data-table), return a reference containing
          the entire key as reference data.
       For a transaction object let transaction-manager handle it.*/

	if (!somobj) return 0;

	cls=somNewClassReference(SOMDObject);

	b=SOMObject_somIsA(somobj,cls);

	somReleaseClassReference(cls);

	if (!b)
	{
		cls=somNewClassReference(somOS_ServiceBase);
		b=SOMObject_somIsA(somobj,cls);
		somReleaseClassReference(cls);

		if (b)
		{
			/* get key */
			/* get class name */
			/* get interface repository */
			/* get interface */
			/* get current implementation */
			/* call SOMOA_create_constant */
		}
	}

	return somOS_Server_parent_SOMDServer_somdRefFromSOMObj(somSelf,ev,somobj);
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_somdserv_somdSOMObjFromRef(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMDObject SOMSTAR objref)
{
	/* RETURNS OBJECT_NIL iff object cannot be successfully created
	   If it is SOM_ref then let SOMDServer handle it
       If the reference is not in somOS::Server family, return OBJECT_NIL
       If the reference is a transaction reference, let TR-Manager handle it
       If there is an entry in reference-data-table then
            return object-ptr stored in it
       Otherwise, verify that it is in persistent store, create a new
            instance using somNewNoInit, init_for_object_reactivation,
            reinit, and add to reference-data-table.
	*/

	return somOS_Server_parent_SOMDServer_somdSOMObjFromRef(somSelf,ev,objref);
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_somdserv_somdCreateObj(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ Identifier objclass,
	/* in */ corbastring hints)
{
    /* If the input class has descended from ServiceBase
       THEN call somNewNoInit; init_for_object_creation()
        ELSE let SOMDServer handle it.
		*/

	return somOS_Server_parent_SOMDServer_somdCreateObj(somSelf,ev,objclass,hints);
}

SOM_Scope void SOMLINK somos_somdserv_somdDeleteObj(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR somobj)
{
    /* If it is a SOMDObject or not a ServiceBase object
       THEN let SOMDServer handle it
       ELSE uninit_for_object_destruction(); somDestruct()
	*/

	somOS_Server_parent_SOMDServer_somdDeleteObj(somSelf,ev,somobj);
}


SOM_Scope SOMObject SOMSTAR SOMLINK somos_servbase_GetInstanceManager(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
   /* - Returns the server object which is either 
        an instance somOS::Server,
        OR  an instance of SOMDServer,
        OR  OBJECT_NIL
        This method is usually not overriden. 
        This method allows clients to get a reference to the server
        object. It is introduced as somdFindServer... methods have been
        deprecated.
		*/
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	if (SOMD_ServerObject)
	{
		return SOMDServer_somDuplicateReference(SOMD_ServerObject);
	}

	return 0;
}

SOM_Scope void SOMLINK somos_servbase_reinit(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev,
	/* in */ somOS_ServiceBase_metastate_t *meta_data)
{
/* - somOS::Server calls this method when (1) an object is reactivated
    (in somdSOMObjFromRef()) and (2) somOS::Server is asked to
    restore_metastate().
   - Services should override it do reinitialization using meta_data.
   - reinitializes the identity metadata */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(meta_data)
}

SOM_Scope void SOMLINK somos_servbase_capture(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev,
	/* inout */ somOS_ServiceBase_metastate_t *meta_data)
{
   /* - somos::Server call this method from store_metastate().
      - Services should override this method to supply proper metadata.
      - Capture() must free typecode in
        meta_data._buffer[service_id].service_metastate._type
        and data in ...._buffer prior to filling it using SOMFree
      - Allocates memory to store metastate. It is somOS::Server's
        resposibility to free the metastate returned by services.
      - captures identity metadata
	*/
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(meta_data)
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_servbase_init_for_object_creation(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
    /* somOS::somOS::Server calls this method from somdCreateObj().
       - Services should override this method to do proper initialization
         at the time of object creation. (if necessary)
       - it initializes the data for CosObjectIdentity
	   */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_servbase_init_for_object_reactivation(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
    /* - somOS::Server calls this method when an object is reactivated
       in somdSOMObjFromRef() (but prior to calling reinit())
       - Services should override this method to do proper initialization
        at the time of object reactivation. (if necessary) */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_servbase_init_for_object_copy(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
    /* Abstract method.
       - Services should override this method to do proper initialization
          at the time of object copy. (if necessary)
		  */

	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope void SOMLINK somos_servbase_uninit_for_object_move(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
    /* Abstract method.
		 - Services should override this method to do proper uninitialization
       at the time of object move. (if necessary) */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK somos_servbase_uninit_for_object_passivation(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
    /* - Asks somOS::Server to remove an entry from reference-data-table.
       - No exceptions are raised. If there is no entry in the
         reference-data-table then the operation is considered to be successful.
       - somOS::Server calls this method when an object is passivated
         (passivate_object()).
       - Services should override this method to do proper uninitialization
         at the time of object passivation. (if necessary)
		 */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK somos_servbase_uninit_for_object_destruction(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
     /* - somOS::Server calls this method when a properly initialized object
          is destroyed. In particular, from somdDeleteObj().
        - Services should override this method to do proper uninitialization
          at the time of object destruction. (if necessary) */
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope CosObjectIdentity_ObjectIdentifier SOMLINK somos_servbase__get_constant_random_id(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope boolean SOMLINK somos_servbase_is_identical(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosObjectIdentity_IdentifiableObject SOMSTAR other_object)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(other_object)

	return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_servbasepref_init_for_object_creation(
	somOS_ServiceBasePRef SOMSTAR somSelf,
	Environment *ev)
{
    /* - somOS::Server calls this method from somdCreateObj().
       - Performs initilialization for persistent objects by calling
         init_for_object_creation() on parent and make_persistent_ref()
         on somOS::server.
       - Services should override this method to do proper
         initialization at the time of object creation. (if necessary)
		 */

	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope void SOMLINK somos_servbasepref_uninit_for_object_destruction(
	somOS_ServiceBasePRef SOMSTAR somSelf,
	Environment *ev)
{
    /* - somOS::Server calls this method when a properly initialized object
         is destroyed. In particular, from somdDeleteObj().
       - Performs uninitilialization for persistent objects by calling
         uninit_for_object_destruction() on parent and delete_ref()
        on somOS::server.
		*/
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope SOMObject SOMSTAR SOMLINK somos_servbase_corba_init_for_object_reactivation(
	somOS_ServiceBaseCORBA SOMSTAR somSelf,
	Environment *ev)
{
    RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,NO);

	SOM_IgnoreWarning(somSelf);

	return 0;
}

SOM_Scope void SOMLINK somos_somdserv_somdDispatchMethod(
	somOS_Server SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR somobj,
	/* out */ somToken *retValue,
	/* in */ somId methodId,
	/* in */ va_list ap)
{
	somOS_Server_parent_SOMDServer_somdDispatchMethod(somSelf,ev,
			somobj,retValue,methodId,ap);
}

SOM_Scope octet * SOMLINK somos_servbase__get_som_uuid(
	somOS_ServiceBase SOMSTAR somSelf,
	Environment *ev)
{
	somOS_ServiceBaseData *somThis=somOS_ServiceBaseGetData(somSelf);
	return somThis->som_uuid;
}
