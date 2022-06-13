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

#ifdef _PLATFORM_MACINTOSH_
	#include <Devices.h>
	#include <ENET.h>
#endif

#include <rhbsomd.h>
#include <rhbgiops.h>

#ifdef _WIN32
	#include <process.h>
#endif

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif

char *somd_dupl_string(const char *p)
{
	if (p)
	{
		char *q;
		if (*p)
		{
			int i=(int)strlen(p)+1;

			q=SOMMalloc(i);

			if (q)
			{
				memcpy(q,p,i);
			}

#ifdef _DEBUG
/*			if (!strcmp(q,"::Repository"))
			{
				somPrintf("%s=[%p],[%p]\n",q,p,q);
			}
*/
#endif

			return q;
		}
		
		q=SOMMalloc(1);

		if (q)
		{
			*q=0;

			return q;
		}
	}

	return 0;
}

#ifdef USE_THREADS
	struct rhbmutex_t somd_mutex
#	ifdef RHBMUTEX_INIT_DATA
		=RHBMUTEX_INIT_DATA
#	endif
		;
#endif

#ifdef _WIN32
int somd_DllMain(void *pv,int reason,void*extra)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	#if defined(USE_THREADS) && !defined(RHBMUTEX_INIT_DATA)
		rhbmutex_init(&somd_mutex);
	#endif
		break;
	case DLL_PROCESS_DETACH:
	#if defined(USE_THREADS) && !defined(RHBMUTEX_INIT_DATA)
		rhbmutex_uninit(&somd_mutex);
	#endif
		break;
	}
	return 1;
}
#endif
