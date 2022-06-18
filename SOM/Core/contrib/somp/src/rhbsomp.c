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

#include <rhbsomp.h>

SOMInitModule_begin(somp)

	SOMInitModule_new(SOMPFileMediaAbstract);
	SOMInitModule_new(SOMPAsciiMediaInterface);
	SOMInitModule_new(SOMPBinaryFileMedia);
	SOMInitModule_new(SOMPAscii);
	SOMInitModule_new(SOMPBinary);
	SOMInitModule_new(SOMPIOGroupMgrAbstract);
	SOMInitModule_new(SOMPMediaInterfaceAbstract);
	SOMInitModule_new(SOMPNameSpaceMgr);
	SOMInitModule_new(SOMPObjectSet);
	SOMInitModule_new(SOMPIdAssigner);
	SOMInitModule_new(SOMPIdAssignerAbstract);
	SOMInitModule_new(SOMPKeyedSet);
	SOMInitModule_new(SOMPEncoderDecoderAbstract);
	SOMInitModule_new(SOMPAttrEncoderDecoder);
	SOMInitModule_new(SOMPIOGroup);
	SOMInitModule_new(SOMPPersistentObject);
	SOMInitModule_new(SOMPPersistentId);
	SOMInitModule_new(SOMPPersistentStorageMgr);

SOMInitModule_end

#ifdef _PLATFORM_WIN32_
static HMODULE SOMP_hModule;
HMODULE SOMP_GetModuleHandle(void)
{
	return SOMP_hModule;
}
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
	RHBOPT_unused(extra)
	RHBOPT_unused(hInst)

 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
				SOMP_hModule=hInst;
/*				{
					char buf[1024];
					GetModuleFileName(hInst,buf,sizeof(buf));
                    somPrintf("(%s)",buf);
				}*/
				return DisableThreadLibraryCalls(hInst);
	        case DLL_PROCESS_DETACH:
				/* this is unwise because of
					the waiting on other threads */
			/*	{
					SOMD_Uninit(0);
				}*/
            	return 1;
			case DLL_THREAD_DETACH:
				return 1;
	}

   	return 1;
}
#endif

#if defined(_PLATFORM_MACINTOSH_) && GENERATINGCFM
void SOMP_CFM_term(void);
RHBOPT_CFM_init(SOMP,initBlockPtr)
{
	return 0;
}

void SOMP_CFM_term(void)
{
}
#endif

