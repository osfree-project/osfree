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
#include <rhbsomos.h>

SOMEXTERN int SOMLINK somos_init_persist_dbs(char *impl_alias, Environment *ev)
{
	RHBOPT_unused(ev)

	somPrintf("somos_init_persist_dbs(%s)\n",impl_alias);

	return 0;
}

SOMEXTERN void SOMLINK somos_uninit_persist_dbs(char *impl_alias, Environment *ev)
{
	RHBOPT_unused(ev)

	somPrintf("somos_uninit_persist_dbs(%s)\n",impl_alias);
}

SOMEXTERN int SOMLINK somos_getopt(int argc, char **argv, char *opts)
{
	RHBOPT_unused(argc)
	RHBOPT_unused(argv)
	RHBOPT_unused(opts)

	somPrintf("somos_getopt(%s)\n",opts);

	return 0;
}

SOMEXTERN void SOMLINK somos_exit(int exit_code)
{
	RHBOPT_unused(exit_code)
}

SOMEXTERN void SOMLINK somos_setup(void)
{
#ifndef _PLATFORM_MACINTOSH_
	somOS_ServerNewClass(
			somOS_Server_MajorVersion,
			somOS_Server_MinorVersion);
#endif
}

SOMEXTERN void SOMLINK somos_init_services(boolean initialize_mode)
{
	somPrintf("somos_init_services(%d)\n",(int)initialize_mode);
}

SOMEXTERN void SOMLINK somos_init_services_afterimpl(boolean initialize_mode)
{
	somPrintf("somos_init_services_afterimpl(%d)\n",(int)initialize_mode);
}

SOMEXTERN void SOMLINK somos_register_exit_callback(somOS_service_id_e somos_service_id, 
						    somosExitCallbackProc *proc,
						    void *client_data)
{
	RHBOPT_unused(somos_service_id)
	RHBOPT_unused(client_data)
	RHBOPT_unused(proc)
}

SOMEXTERN void SOMLINK somos_init_logging(void)
{
	somPrintf("somos_init_logging\n");
}

