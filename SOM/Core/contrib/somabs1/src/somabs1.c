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
#	include <windows.h>
#endif

#include <rhbsomex.h>

#ifndef USE_APPLE_SOM
#	include <somref.h>
#endif

#define SOM_Module_omgidobj_Source
#include <omgidobj.h>

SOMInitModule_begin(somabs1)

   SOMInitModule_new(CosObjectIdentity_IdentifiableObject);

SOMInitModule_end

#if defined(_PLATFORM_MACINTOSH_) && GENERATINGCFM
RHBOPT_CFM_init(SOMABS1,initBlockPtr)
{
	if (initBlockPtr)
	{
		return 0;
	}

	return -1;
}
#endif

#ifdef _WIN32
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
/*				{
					char buf[1024];
					GetModuleFileName(hInst,buf,sizeof(buf));
                    somPrintf("(%s)",buf);
				}*/
				return DisableThreadLibraryCalls(hInst);
	        case DLL_PROCESS_DETACH:
           		return 1;
	}

   	return 1;
}
#endif
