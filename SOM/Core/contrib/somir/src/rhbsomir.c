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

#include <rhbsomir.h>
#include <rhbiniut.h>

#if !defined(USE_APPLE_SOM)
SOMInitModule_begin(somir)

	SOM_IgnoreWarning(major);
	SOM_IgnoreWarning(minor);
	SOM_IgnoreWarning(classname);

	SOMInitModule_new(Contained);
	SOMInitModule_new(Container);
	SOMInitModule_new(InterfaceDef);
	SOMInitModule_new(OperationDef);
	SOMInitModule_new(ExceptionDef);
	SOMInitModule_new(ParameterDef);
	SOMInitModule_new(ModuleDef);
	SOMInitModule_new(TypeDef);
	SOMInitModule_new(AttributeDef);
	SOMInitModule_new(ConstantDef);
	SOMInitModule_new(M_Repository);
	SOMInitModule_new(Repository);

SOMInitModule_end
#endif

#if defined(USE_THREADS) && defined(USE_PTHREADS)
static void somir_dll_init_once(void)
{
#ifdef RHBMUTEX_INIT_DATA
#else
	RHBMUTEX_INIT(&somir_crit);
#endif
}
#endif

void somir_dll_init(void)
{
#ifdef USE_THREADS
#	ifdef USE_PTHREADS
		static pthread_once_t once=RHBOPT_PTHREAD_ONCE_INIT;
		pthread_once(&once,somir_dll_init_once);
#	else
		InitializeCriticalSection(&somir_crit);
#	endif
#endif
}

void somir_dll_term(void)
{
#ifdef USE_THREADS
#	if !defined(USE_PTHREADS)
		DeleteCriticalSection(&somir_crit);
#	endif
#endif
}

#ifdef _WIN32
void *SOMIR_hModule;
#if defined(USE_THREADS)
static CRITICAL_SECTION win32env_SOMIR_critical;
#endif
static char *win32env_SOMIR;
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
   	{
   	case DLL_PROCESS_ATTACH:
		SOMIR_hModule=hInst;
#ifdef USE_THREADS
			InitializeCriticalSection(&win32env_SOMIR_critical);
#	ifdef USE_PTHREADS
#	else
			somir_dll_init();
#	endif
#endif
		return 1;
	case DLL_PROCESS_DETACH:
		if (win32env_SOMIR)
		{
			char *p=win32env_SOMIR;
			win32env_SOMIR=NULL;
			LocalFree(p);
		}
#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#	else
		somir_dll_term();
#	endif
		DeleteCriticalSection(&win32env_SOMIR_critical);
#endif
        return 1;
	}

	SOM_IgnoreWarning(extra);

   	return 1;
}
#endif

SOM_IMPORTEXPORT_somir char * SOMLINK irGetFileNames(void)
{
#ifdef _WIN32
	#ifdef UNDER_CE
		return "som.ir";
	#else
		#ifdef USE_THREADS
			EnterCriticalSection(&win32env_SOMIR_critical);
		#endif
			if (!win32env_SOMIR)
			{
				static char somir[]="SOMIR";
				DWORD len=GetEnvironmentVariable(somir,NULL,0);
				if (len)
				{
					char *p=LocalAlloc(LMEM_FIXED,len);
					if (p)
					{
						if (GetEnvironmentVariable(somir,p,len))
						{
							win32env_SOMIR=p;
						}
						else
						{
							LocalFree(p);
						}
					}
				}
			}
		#ifdef USE_THREADS
			LeaveCriticalSection(&win32env_SOMIR_critical);
		#endif
			return win32env_SOMIR ? win32env_SOMIR : "som.ir";
	#endif
#else
	#ifdef _PLATFORM_MACINTOSH_
		return NULL;
	#else
		return getenv("SOMIR");
	#endif
#endif
}

SOM_IMPORTEXPORT_somir char * SOMLINK irGetLastFileName(void)
{
	char *p=irGetFileNames();
	char *q=p;

	if (p)
	{
		while (*p)
		{
			if (*p==kSOMIRseparator)
			{
				p++;
				q=p;
			}
			else
			{
				p++;
			}
		}
	}

	return q;
}

