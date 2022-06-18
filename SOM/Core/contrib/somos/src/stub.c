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

#define somAttributePersistence_Persist_Class_Source
#define somOS_Server_Class_Source
#define somOS_ServiceBase_Class_Source
#define somOS_ServiceBaseCORBA_Class_Source
#define somOS_ServiceBasePRef_Class_Source
#define somOS_ServiceBasePRefCORBA_Class_Source

#include <rhbsomex.h>

#ifdef SOMDLLEXPORT
	#define SOM_IMPORTEXPORT_somos SOMDLLEXPORT
#else
	#define SOM_IMPORTEXPORT_somos 
#endif

#include <somstubs.h>
#include <somd.h>

#include <somap.h>
#include <somos.h>
#include <somosutl.h>
#include <rhbsomex.h>

SOMSTUB(somAttributePersistence_Persist)
SOMSTUB(somOS_Server)
SOMSTUB(somOS_ServiceBase)
SOMSTUB(somOS_ServiceBaseCORBA)
SOMSTUB(somOS_ServiceBasePRef)
SOMSTUB(somOS_ServiceBasePRefCORBA)

int SOMLINK somos_init_persist_dbs(char *impl_alias, Environment *ev)
{
	return 0;
}
void SOMLINK somos_uninit_persist_dbs(char *impl_alias, Environment *ev)
{
}
int SOMLINK somos_getopt(int argc, char **argv, char *opts)
{
	return 0;
}

void SOMLINK somos_exit(int exit_code)
{
}

void SOMLINK somos_setup(void)
{
}

void SOMLINK somos_init_services(boolean initialize_mode)
{
}

void SOMLINK somos_init_services_afterimpl(boolean initialize_mode)
{
}

void SOMLINK somos_register_exit_callback(somOS_service_id_e somos_service_id, 
						    somosExitCallbackProc *proc,
						    void *client_data)
{
}

void SOMLINK somos_init_logging(void)
{
}

SOMInitModule_begin(somos)
SOMInitModule_end
