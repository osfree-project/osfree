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

#ifdef _PLATFORM_MACINTOSH_
	#include <CodeFragments.h>
#endif

/***********************************
 * environment variables used
 *	SOMBASE
 *	SOMENV=$SOMBASE/etc/somenv.ini
 *	SOMDDIR=$SOMBASE/etc/dsom
 */

#include <rhbopt.h>

#ifdef _WIN32
#	include <windows.h>
#	include <rpc.h>
#else
#	include <rhbendia.h>
#	ifdef HAVE_SYS_UUID_H
#		include <sys/uuid.h>
#	endif
#	ifdef HAVE_UUID_UUID_H
#		include <uuid/uuid.h>
#	endif
#	ifdef HAVE_UUID_H
#		include <uuid.h>
#	endif
#endif

#include <rhbsomex.h>
#include <rhbsomu.h>
#include <rhbiniut.h>
#include <somuutil.h>
#include <somproxy.h>
#include <snglicls.h>
#include <somsid.h>
#include <somida.h>
#include <somuutil.h>

static char *rhbsomu_env;
static struct rhbinifile *iniFile;

#if defined(_WIN32) || defined(__OS2__)
static char rhbsomu_base[]="c:\\som";
static char rhbsomu_somini[]="\\etc\\somenv.ini";
#else
static char rhbsomu_base[]="/usr/lpp/somtk";
static char rhbsomu_somini[]="/etc/somenv.ini";
#endif

#ifdef _WIN32
#	ifdef getenv
#		undef getenv
#	endif
#	define getenv	rhbsomu_getenv
struct env
{
	struct env *next;
	char *value;
	char name[1];
};
static struct env *rhbsomu_envlist;
static CRITICAL_SECTION rhbsomu_crit;
static char *getenv(const char* name)
{
	char *result=NULL;

	EnterCriticalSection(&rhbsomu_crit);

	if (rhbsomu_envlist)
	{
		struct env *p=rhbsomu_envlist;

		while (p)
		{
			if (!strcmp(p->name,name))
			{
				result=p->value;
				break;
			}

			p=p->next;
		}
	}

	if (!result)
	{
		DWORD len=GetEnvironmentVariable(name,NULL,0);

		if (len++)
		{
			size_t len2=strlen(name)+1;
			struct env *p=LocalAlloc(LMEM_FIXED,sizeof(p[0])+len2+len);
			memcpy(p->name,name,len2);
			p->value=p->name+len2;
			len=GetEnvironmentVariable(name,p->value,len);
			p->value[len]=0;
			p->next=rhbsomu_envlist;
			rhbsomu_envlist=p;
			result=p->value;
		}
	}

	LeaveCriticalSection(&rhbsomu_crit);

	return result;
}
static void rhbsomu_freeenv(void)
{
	while (rhbsomu_envlist)
	{
		struct env *p=rhbsomu_envlist;
		rhbsomu_envlist=p->next;

		LocalFree(p);
	}
}
#endif

#ifdef USE_THREADS
#	ifdef USE_PTHREADS
		static pthread_mutex_t rhbsomu_mutex=RHBOPT_PTHREAD_MUTEX_INITIALIZER;
#		define RHBSOMU_ENTER		pthread_mutex_lock(&rhbsomu_mutex);
#		define RHBSOMU_LEAVE		pthread_mutex_unlock(&rhbsomu_mutex);
#	else
#		define RHBSOMU_ENTER    	EnterCriticalSection(&rhbsomu_crit); 
#		define RHBSOMU_LEAVE    	LeaveCriticalSection(&rhbsomu_crit); 
#	endif
#else
#		define RHBSOMU_ENTER    	
#		define RHBSOMU_LEAVE    	
#endif

SOMEXTERN char * SOMLINK somutgetenv(char *envvar, char *stanza)
{
	char *result=NULL;

	RHBSOMU_ENTER

	if (!iniFile)
	{
		boolean doFree=0;
		char *env=rhbsomu_env;

		if (!env)
		{
			env=getenv("SOMENV");

			if (!env)
			{
				char *base=getenv("SOMBASE");
				size_t i=strlen(rhbsomu_somini)+2;

				if (!base)
				{
					base=rhbsomu_base;
				}

				i+=strlen(base);

				doFree=1;
				env=malloc(i);
				strncpy(env,base,i);
				strncat(env,rhbsomu_somini,i);
			}
		}

		iniFile=rhbinifile(env);

		if (doFree)
		{
			free(env);
		}
	}

	if (iniFile)
	{
		result=(char *)iniFile->lpVtbl->GetValue(iniFile,envvar,stanza);
	}

	RHBSOMU_LEAVE

	return result;
}

SOMEXTERN char * SOMLINK somutgetshellenv(char *envvar, char *stanza)
{
	char *val=getenv(envvar);

	if (!val)
	{
		val=somutgetenv(envvar,stanza);
	}

	return val;
}

SOMEXTERN void   SOMLINK somutresetenv(char *newenv)
{
	RHBSOMU_ENTER

	{
		char *p=rhbsomu_env;
		struct rhbinifile *f=iniFile;

		rhbsomu_env=NULL;
		iniFile=NULL;

		if (p)
		{
			free(p);
		}

		if (f)
		{
			f->lpVtbl->Release(f);
		}

		if (newenv)
		{
			rhbsomu_env=strdup(newenv);
		}
	}

#ifdef _WIN32
	rhbsomu_freeenv();
#endif

	RHBSOMU_LEAVE
}

#define GETPATH_LEN		256

#if defined(_WIN32) || defined(__OS2__)
static const char szSOMBASE_DEFAULT[]="c:\\som";
static const char dirSep='\\';
#else
static const char szSOMBASE_DEFAULT[]="/usr/lpp/somtk";
static const char dirSep='/';
#endif

static void addSlash(char *p,size_t len)
{
	size_t i=strlen(p);

	if (i && (i < (len-1)))
	{
		char c=p[i-1];
		if (c=='/') return;
#if defined(_WIN32) || defined(__OS2__)
		if (c=='\\') return;
#endif
	}

	p[i++]=dirSep;
	p[i]=0;
}

SOMEXTERN int SOMLINK somutgetpath(char *path)
{
	if (path)
	{
		char *p=somutgetenv("SOMDDIR","[somd]");

		if (p)
		{
			strncpy(path,p,GETPATH_LEN);
		}
		else
		{
#ifdef _WIN32
			char buf[256];
			DWORD dw=GetEnvironmentVariable("SOMBASE",buf,sizeof(buf)-1);

			if (dw)
			{
				buf[dw]=0;
				p=buf;
			}
#else
			p=getenv("SOMBASE");
#endif

			if (p)
			{
				strncpy(path,p,GETPATH_LEN);
			}
			else
			{
				strncpy(path,szSOMBASE_DEFAULT,GETPATH_LEN);
			}

			addSlash(path,GETPATH_LEN);

			strncat(path,"etc",GETPATH_LEN);

			addSlash(path,GETPATH_LEN);

			strncat(path,"dsom",GETPATH_LEN);
		}

		addSlash(path,GETPATH_LEN);
	}
	
	return 0;
}

SOMEXTERN int SOMLINK somCreateUUID(octet *buf)
{
	int rc=0;
#ifdef _WIN32
	UUID id;

	rc=UuidCreate(&id);

	if (!rc)
	{
		*buf++=(octet)(id.Data1 >> 24);
		*buf++=(octet)(id.Data1 >> 16);
		*buf++=(octet)(id.Data1 >> 8);
		*buf++=(octet)(id.Data1);
		*buf++=(octet)(id.Data2 >> 8);
		*buf++=(octet)(id.Data2);
		*buf++=(octet)(id.Data3 >> 8);
		*buf++=(octet)(id.Data3);
		memcpy(buf,id.Data4,8);
	}
#else
#	ifdef HAVE_UUID_GENERATE
	uuid_t id;
	if (sizeof(id)==16)
	{
		uuid_generate(id);

		memcpy(buf,id,sizeof(id));
	}
	else
	{
		rc=1;
	}
#	else
	uuid_t id;
	uint32_t s=1;

	if (sizeof(id)==16)
	{
		uuid_create(&id,&s);

		if (s)
		{
			rc=(int)s;
		}
		else
		{
			memcpy(buf,&id,sizeof(id));
		}
	}
	else
	{
		rc=1;
	}
#	endif
#endif

	return rc;
}

static char hexmap_lower[]="0123456789abcdef";
static char hexmap_upper[]="0123456789ABCDEF";

static octet hexchar(char c)
{
	octet i=sizeof(hexmap_lower);

	while (i--)
	{
		if (hexmap_lower[i]==c) return i;
		if (hexmap_upper[i]==c) return i;
	}

	return 0xf0;
}

static int pull_hex(char *str)
{
	if (*str)
	{
		octet higher=hexchar(*str++);

		if (*str)
		{
			octet lower=hexchar(*str);

			if (!((higher|lower)&0xf0))
			{
				return (higher << 4)|lower;
			}
		}
	}

	return -1;
}

static octet uuid_seq[]={4,4,1,1,6,0};

SOMEXTERN int SOMLINK somStringToUUID(char *strid,octet *id)
{
	if (strid && id)
	{
		octet buf[16];
		octet *p=buf;
		int i=0;

		while (*strid)
		{
			int j=uuid_seq[i++];

			while (j--)
			{
				int val=pull_hex(strid); 

				if (val < 0) 
				{
					return 0;
				}

				*p++=(octet)val;
				strid+=2;
			}

			if (uuid_seq[i])
			{
				if ('-' != *strid++) 
				{
					return 0;
				}
			}
			else
			{
				if (*strid) 
				{
					return 0;
				}

				break;
			}
		}

		memcpy(id,buf,sizeof(buf));

		return 1;
	}

	return 0;
}

SOMEXTERN char * SOMLINK somUUIDToString(octet *id,char *strid)
{
	if (strid && id)
	{
		char *retval=strid;
		int i=0;

		while (strid)
		{
			int j=uuid_seq[i++];

			if (j)
			{
				while (j--)
				{
					octet b=*id++;

					*strid++=hexmap_lower[0xf & (b >> 4)];
					*strid++=hexmap_lower[0xf & b];
				}

				if (uuid_seq[i])
				{
					*strid++='-';
				}
			}
			else
			{
				break;
			}
		}

		*strid=0;

		return retval;
	}

	return NULL;
}

SOMEXTERN int SOMLINK somCompareUUIDStrings(char *id1,char *id2)
{
	if (id1 && id2)
	{
		int retval=-1;
		do
		{
			char c1=*id1++;
			char c2=*id2++;
/*			if ((c1 >='a')&&(c1 <='z')) c1+=('A'-'a');
			if ((c2 >='a')&&(c2 <='z')) c2+=('A'-'a');*/

			retval=c1-c2;

			if (!c1) break;

		} while (!retval);

		return retval;
	}

	return -1;
}

SOMInitModule_begin(somu)

	SOMInitModule_new(SOMMSingleInstance);
	SOMInitModule_new(SOMMProxyForObject);
	SOMInitModule_new(SOMMProxyFor);
	SOMInitModule_new(SOMUTStringId);
	SOMInitModule_new(SOMUTId);

SOMInitModule_end

#ifdef _PLATFORM_WIN32_
static HMODULE hSOMUdll;
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
				hSOMUdll=hInst;
				InitializeCriticalSection(&rhbsomu_crit);
				SOMTHRD_init();
           		return 1;
			case DLL_THREAD_DETACH:
				SOMTHRD_thread_dead();
				return 1;
	        case DLL_PROCESS_DETACH:
				SOMTHRD_term();
				rhbsomu_freeenv();
				DeleteCriticalSection(&rhbsomu_crit);
           		return 1;
	}
	SOM_IgnoreWarning(extra);
   	return 1;
}
#endif

#ifdef _PLATFORM_MACINTOSH_
#define RHB_NewClass(x)
RHBOPT_CFM_init(SOMU,initBlockPtr)
{
	if (initBlockPtr)
	{
/*		RHB_NewClass(SOMMBeforeAfter);*/
		RHB_NewClass(SOMMProxyFor);
		RHB_NewClass(SOMMProxyForObject);
		RHB_NewClass(SOMMSingleInstance);
		RHB_NewClass(SOMUTId);
		RHB_NewClass(SOMUTStringId);

		return 0;
	}

	return -1;
}
#endif

