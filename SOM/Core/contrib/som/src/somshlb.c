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

/* $Id$ */

#include <rhbopt.h>

#ifdef _WIN32
	#include <windows.h>
#endif

#include <somkern.h>
#include <somkpath.h>

#include <rhbsomex.h>

#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
	#include <errno.h>
#endif

#ifdef HAVE_SIGNAL_H
	#include <signal.h>
#endif

#ifndef BUILD_STATIC
	#ifdef HAVE_SYS_LDR_H
		/* AIX */
		#include <sys/ldr.h>
	#else
		#ifdef HAVE_DLFCN_H
			/* Solaris, ELF et al */
			#include <dlfcn.h>
			#include <shlbtest.h>
		#else
			#ifdef HAVE_DL_H
				/* HPUX */
				#include <dl.h>
			#endif
		#endif
	#endif
#endif

#ifdef _PLATFORM_MACINTOSH_
	#include <Errors.h>
	#define HAVE_DLFCN_H
#endif

SOMInitModule_begin(som)
SOMInitModule_end


#if defined(_WIN32) && defined(UNDER_CE)
static wchar_t *toWideString(const char *p,void *buf,size_t buflen)
{
	wchar_t *wch=NULL;
	if (p)
	{
		int len=strlen(p);
		int i=MultiByteToWideChar(CP_ACP,0,p,len,buf,(buflen/sizeof(wch[0]))-1);
		wch=buf;
		wch[i]=0;

	}
	return wch;
}
#	define toString(p,b,l)		toWideString(p,b,l)
#else
#	define toString(p,b,l)		(p)
#endif

static int SOMLINK SOMKERN_deleteModule(somToken m)
{
#ifndef BUILD_STATIC
	somStartCriticalSection();

#ifdef _PLATFORM_WIN32_
	FreeLibrary(m);
#else
	#ifdef HAVE_SYS_LDR_H
		unload(m);
	#else
		#ifdef HAVE_DLFCN_H
			dlclose(m);
		#else
			#ifdef HAVE_DL_H
				shl_unload(m);
			#endif
		#endif
	#endif
#endif

	somEndCriticalSection();
#endif

	return 0;
}

#ifndef BUILD_STATIC
static void *loadModule(char *fileName)
{
void *hMod=NULL;
#if defined(_WIN32) && defined(UNICODE)
	wchar_t tmp[260];
#endif

#ifdef _PLATFORM_WIN32_
	hMod=LoadLibrary(toString(fileName,tmp,sizeof(tmp)));
#else
	#ifdef HAVE_SYS_LDR_H
		hMod=(void *)load(fileName,1,NULL);
	#else
		#ifdef HAVE_DLFCN_H
			hMod=dlopen(fileName,
				0
			#ifdef RTLD_GLOBAL
				|RTLD_GLOBAL
			#endif
			#ifdef RTLD_NOW
				|RTLD_NOW
			#endif
				);
		#else
			#ifdef HAVE_DL_H
				hMod=shl_load(fileName,BIND_IMMEDIATE|BIND_VERBOSE,0L);
			#endif
		#endif
	#endif
#endif

	return hMod;
}

struct SOMKERN_loadModule
{
	#ifdef _PLATFORM_WIN32_
		HMODULE hMod;
	#else
		#if defined(HAVE_DLFCN_H) || defined(HAVE_SYS_LDR_H)
			void *hMod;
		#else
			#ifdef HAVE_DL_H
				shl_t hMod;
			#else
				void *hMod;
			#endif
		#endif
	#endif
};
RHBOPT_cleanup_begin(SOMKERN_loadModule_cleanup,pv)

struct SOMKERN_loadModule *data=pv;

	if (data->hMod) SOMKERN_deleteModule(data->hMod);

RHBOPT_cleanup_end
#endif

static int SOMLINK SOMKERN_loadModule(char *  className ,
                                        char * fileName,
                                        char * functionName,
                                        long   majorVersion,
                                        long   minorVersion,
                                        somToken * modHandle)
{
#ifndef BUILD_STATIC
	RHBOPT_volatile int rc=-1;
	struct SOMKERN_loadModule data={NULL};

	RHBOPT_cleanup_push(SOMKERN_loadModule_cleanup,&data);

	somStartCriticalSection();

	*modHandle=NULL;

	data.hMod=loadModule(fileName);

	if (!data.hMod)
	{
		const char **p=somkern_pathList;
		size_t j=strlen(fileName);

		while (*p)
		{
			const char *q=*p++;
			size_t i=strlen(q);
			char buf[256];

/*			somPrintf("dlopen(%s) failed with %s, now trying path %s\n",
					fileName,dlerror(),q);*/

			if ((i+j+2)<sizeof(buf))
			{
				memcpy(buf,q,i);
#ifdef _WIN32
				buf[i++]='\\';
#else
				buf[i++]='/';
#endif
				memcpy(buf+i,fileName,j+1);					

/*				somPrintf("trying %s\n",buf); */

				data.hMod=loadModule(buf);

				if (data.hMod) 
				{
/*					somPrintf("found %s\n",buf); */
					break;
				}
			}
		}
	}

	if (!data.hMod)
	{
		somEndCriticalSection();
#if 0
		#ifdef HAVE_SYS_LDR_H
			somPrintf("load(%s) failed with %d\n",
					fileName,errno);
		#else
			#ifdef HAVE_DLFCN_H
				somPrintf("dlopen(%s) failed with %s\n",
					fileName,dlerror());
			#else
				#ifdef HAVE_DL_H
					somPrintf("shl_load(%s) failed with %d\n",
						fileName,errno);
				#endif
			#endif
		#endif
#endif
	}
	else
	{
#if defined(_WIN32) && defined(UNICODE)
		wchar_t tmp[260];
#endif
		typedef void (SOMLINK *somTP_SOMInitModule)(long,long,char *);
		/* assignment here is to confirm ABI from rhbsomex.h */
		somTP_SOMInitModule e=
#if defined(BUILD_STATIC) || defined(HAVE_MACHO_DYLD_H) || defined(__ELF__)
			som_SOMInitModule;
#else
			SOMInitModule;
#endif

/*		somPrintf("dlopen(%s) returned %p\n",fileName,data.hMod);*/

#ifdef _PLATFORM_WIN32_
		e=(somTP_SOMInitModule)GetProcAddress(data.hMod,toString(functionName,tmp,sizeof(tmp)));
#else
	#ifdef HAVE_SYS_LDR_H
			/* this is for AIX */
			e=(somTP_SOMInitModule)data.hMod;
	#else
		#ifdef HAVE_DLFCN_H
			e=(somTP_SOMInitModule)dlsym(data.hMod,functionName);
		#else
			#ifdef HAVE_DL_H
				void *entrypoint=NULL;
				if (!shl_findsym(&data.hMod,functionName,TYPE_PROCEDURE,&entrypoint))
				{
					e=(somTP_SOMInitModule)entrypoint;
				}
				else
				{
					e=NULL;
				}
			#else
				e=NULL;
			#endif
		#endif
	#endif
#endif

		somEndCriticalSection();

		if (e)
		{
/*			somPrintf("Calling '%s' in '%s'\n",functionName,fileName);*/

			e(majorVersion,minorVersion,className);

			rc=0;
		}
		else
		{
/*			somPrintf("Failed to get '%s' from '%s'\n",functionName,fileName); */

#ifdef SUPPORT_LOADING_NEWCLASS
			if (className)
			{
				char buf[256];
				typedef SOMClass SOMSTAR (SOMLINK *newClass)(integer4 major,integer4 minor) ;
				newClass e2=NULL;
				char *p=className;
				char *q=buf;

				while (*p)
				{
					if (*p==':')
					{
						*q++='_';
						p++;
						if (*p==':') p++;
					}
					else
					{
						*q++=*p++;
					}
				}

				strncpy(q,"NewClass",sizeof(buf)-(q-buf));

/*				somPrintf("Trying to get '%s' from '%s'\n",buf,fileName);*/

	#ifdef _PLATFORM_WIN32_
				e2=(newClass)GetProcAddress(data.hMod,toString(buf,tmp,sizeof(tmp)));
	#else
		#if defined(HAVE_DLFCN_H)
				e2=(newClass)dlsym(data.hMod,buf);
		#endif
	#endif
				if (e2)
				{
					if (e2(majorVersion,minorVersion))
					{
						rc=0;
					}
				}
			}
	#endif
		}
	}

	if (!rc)
	{
		*modHandle=data.hMod;
		data.hMod=NULL;
	}

	RHBOPT_cleanup_pop();

	return rc;
#else
	*modHandle=NULL;

	return -1;
#endif
}

static char * SOMLINK SOMKERN_classInitFuncName(void)
{
#ifdef USE_DLSYM_UNDERSCORE
	return "_SOMInitModule";
#else
	return "SOMInitModule";
#endif
}

SOM_IMPORTEXPORT_som somTD_SOMClassInitFuncName  * SOMDLINK SOMClassInitFuncName=SOMKERN_classInitFuncName;
SOM_IMPORTEXPORT_som somTD_SOMDeleteModule		 * SOMDLINK SOMDeleteModule=SOMKERN_deleteModule;
SOM_IMPORTEXPORT_som somTD_SOMLoadModule		 * SOMDLINK SOMLoadModule=SOMKERN_loadModule;

