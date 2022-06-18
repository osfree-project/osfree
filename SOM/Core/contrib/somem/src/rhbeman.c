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
	#include <windows.h>
	#include <winsock.h>
	#include <process.h>
#endif

#include <time.h>

#include <rhbemani.h>
#include <somobj.h>
#include <emtypes.h>
#include <event.h>
#include <eventmsk.h>
#include <eman.h>
#include <emregdat.h>
#include <clientev.h>
#include <workprev.h>
#include <timerev.h>
#include <stdio.h>
#include <rhbeman.h>

SOMInitModule_begin(somem)

   SOMInitModule_new(SOMEEMan);
   SOMInitModule_new(SOMEEvent);
   SOMInitModule_new(SOMEClientEvent);
   SOMInitModule_new(SOMETimerEvent);
#ifdef RHBEMAN_SINKS
   SOMInitModule_new(SOMESinkEvent);
#endif
   SOMInitModule_new(SOMEWorkProcEvent);
   SOMInitModule_new(SOMEEMRegisterData);

SOMInitModule_end

#ifdef _PLATFORM_WIN32_
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


