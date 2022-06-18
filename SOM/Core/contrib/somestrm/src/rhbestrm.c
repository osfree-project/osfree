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
#endif

#define SOM_Module_omgestio_Source
#define SOM_Module_somestio_Source

#include <rhbsomex.h>

#include <somir.h>
#include <somd.h>

#include <omgestio.h>
#include <somestio.h>
#include <rhbsomex.h>

#if defined(USE_APPLE_SOM)
RHBOPT_CFM_init(SOMESTRM,initBlockPtr)
{
	if (initBlockPtr)
	{
		return 0;
	}
	return -1;
}
#endif

SOMInitModule_begin(somestrm)

	SOMInitModule_new(CosStream_Streamable);
	SOMInitModule_new(CosStream_StreamIO);
	SOMInitModule_new(CosStream_StreamableFactory);
	SOMInitModule_new(somStream_StreamIO);
	SOMInitModule_new(somStream_MemoryStreamIO);

		/* this interface is rather abstract and problematic
			it requires a dependancy on SOMOS which depends on SOMD 
			which depends on SOMESTRM... */

	SOMInitModule_new(somStream_Streamable);

SOMInitModule_end

/* these would go in the DEF file 

		somStream_StreamIOClassData
		somStream_StreamIOCClassData
		somStream_StreamIONewClass
		somStream_MemoryStreamIOCClassData
		somStream_MemoryStreamIOClassData
		somStream_MemoryStreamIONewClass
		somStream_StreamableCClassData
		somStream_StreamableNewClass
		somStream_StreamableClassData

  */

#ifdef _WIN32
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
/*
				{
					char buf[1024];
					GetModuleFileName(hInst,buf,sizeof(buf));
                    somPrintf("(%s)",buf);
				}
*/
				return DisableThreadLibraryCalls(hInst);
	        case DLL_PROCESS_DETACH:
            		return 1;
	}

   	return 1;
}
#endif
