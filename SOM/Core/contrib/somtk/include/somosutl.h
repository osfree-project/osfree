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

#ifndef SOMOSUTL_H
#define SOMOSUTL_H

#ifdef __cplusplus
	#include <som.xh>
	#include <somos.xh>
#else
	#include <som.h>
	#include <somos.h>
#endif /* __cplusplus */

#ifndef SOM_IMPORTEXPORT_somos
	#ifdef SOMDLLIMPORT
		#define SOM_IMPORTEXPORT_somos SOMDLLIMPORT
	#else
		#define SOM_IMPORTEXPORT_somos 
	#endif
#endif /* SOM_IMPORTEXPORT_somos */

#define SOMOS_USAGE_ERROR               100
#define SOMOS_FIND_IMPLDEF_FAILED       101
#define SOMOS_IMPL_IS_READY_FAILED      102
#define SOMOS_CONFIG_FILE_ERROR         103
#define SOMOS_LOG_OPEN_FAILED           104
#define SOMOS_PERSIST_INIT_FAILED       105
#define SOMOS_SECURITY_INIT_FAILED      106
#define SOMOS_SOMD_INIT_FAILED          107
#define SOMOS_REQUEST_LOOP_ERROR        108
#define SOMOS_SERVER_FAILED             109
#define SOMOS_SOMOA_NEW_FAILED          110

#ifdef __cplusplus
extern "C" {
#endif

typedef void SOMLINK somosExitCallbackProc(void *client_data);
SOMEXTERN SOM_IMPORTEXPORT_somos int SOMLINK somos_init_persist_dbs(char *impl_alias, Environment *ev);
SOMEXTERN SOM_IMPORTEXPORT_somos void SOMLINK somos_uninit_persist_dbs(char *impl_alias, Environment *ev);
SOMEXTERN SOM_IMPORTEXPORT_somos int SOMLINK somos_getopt(int argc, char **argv, char *opts);
SOMEXTERN SOM_IMPORTEXPORT_somos void SOMLINK somos_exit(int exit_code);
SOMEXTERN SOM_IMPORTEXPORT_somos void SOMLINK somos_setup(void);
SOMEXTERN SOM_IMPORTEXPORT_somos void SOMLINK somos_init_services(boolean initialize_mode);
SOMEXTERN SOM_IMPORTEXPORT_somos void SOMLINK somos_init_services_afterimpl(boolean initialize_mode);
SOMEXTERN SOM_IMPORTEXPORT_somos void SOMLINK somos_register_exit_callback(somOS_service_id_e somos_service_id, 
						    somosExitCallbackProc *proc,
						    void *client_data); 
SOMEXTERN SOM_IMPORTEXPORT_somos void SOMLINK somos_init_logging(void);

#ifdef PRAGMA_IMPORT_SUPPORTED
	#if PRAGMA_IMPORT_SUPPORTED
		#ifdef BUILD_SOMOS
			#pragma export list somos_init_persist_dbs,somos_uninit_persist_dbs,\
					somos_getopt,somos_exit,somos_setup,\
					somos_init_services,somos_init_services_afterimpl,\
					somos_register_exit_callback,somos_init_logging
		#else
			#pragma import list somos_init_persist_dbs,somos_uninit_persist_dbs,\
					somos_getopt,somos_exit,somos_setup,\
					somos_init_services,somos_init_services_afterimpl,\
					somos_register_exit_callback,somos_init_logging
		#endif
	#endif /* PRAGMA_IMPORT_SUPPORTED */
#endif /* PRAGMA_IMPORT_SUPPORTED */

#ifdef __cplusplus
}
#endif

#endif /* SOMOSUTL_H */


