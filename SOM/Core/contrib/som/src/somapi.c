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
#ifdef HAVE_STRINGS_H
#	include <strings.h>
#endif


/* in somAddStaticMethod, if the redispatch stub is -1 then
	a pointer to a struct of type somApRdInfo is passed as
	the applyStub, otherwise redispatchStub and applStub are
	taken as given

	in somAddDynamicMethod, an actual applyStub must be given
	redispatch stubs for dynamic methods are not supported
  */

boolean SOMLINK somApply(SOMObject SOMSTAR somSelf,
                                somToken *retVal,
                                somMethodDataPtr md,
                                va_list ap)
{
	typedef void (SOMLINK *applyStubPtr)(SOMObject SOMSTAR,
					somToken,somMethodProc,va_list);

	if (-1L == (long)(md->mToken->defined.redispatchStub))
	{
		somApRdInfo *info=(void *)md->mToken->defined.applyStub;

		if (info)
		{
			if (info->apStub)
			{
				((applyStubPtr)(info->apStub))(somSelf,retVal,md->method,ap);						

				return 1;
			}

#ifdef SOM_METHOD_STUBS
			((applyStubPtr)(&(md->mToken->jumper.apply)))(somSelf,retVal,md->method,ap);
			return 1;
#endif
		}

		RHBOPT_ASSERT(!info)
	}
	else
	{
		if (md->mToken->defined.applyStub)
		{
			((applyStubPtr)(md->mToken->defined.applyStub))(somSelf,retVal,md->method,ap);						

			return 1;
		}

	}

	RHBOPT_ASSERT(!md)

	return 0;
}

SOMClass SOMSTAR SOMLINK somGetClassFromMToken(somMToken mToken)
{
	return mToken->classInfoOwner->classObject;
}

int SOMLINK somCompareIds(somId id1, somId id2)
{
	if (id1==id2)
	{
		return 1;
	}

	if (*id1==*id2)
	{
		return 1;
	}

	if (*id1) if (!*id2) return 0;
	if (*id2) if (!*id1) return 0;

	return (!
#ifdef HAVE_STRCASECMP
		strcasecmp(*id1,*id2)
#else
		_stricmp(*id1,*id2)
#endif
		) ? 1 : 0;
}

SOMEXTERN void SOMLINK somCheckArgs(int argc, char **argv)
{
}

static void somAssertFailed(void)
{
#if defined(_WIN32) && defined(_DEBUG)
	#ifdef __asm
		#undef __asm
	#endif
	#ifdef _M_IX86
		__asm int 3
	#else
		RaiseException(
			ERROR_INTERNAL_ERROR,
			EXCEPTION_NONCONTINUABLE,0,NULL);
	#endif
#else
	#ifdef USE_THREADS
		#ifdef USE_PTHREADS
			#ifdef PTHREAD_CANCELED
				pthread_exit((void *)PTHREAD_CANCELED);
			#else
				pthread_exit((void *)-1);
			#endif
		#else
				RaiseException(
					ERROR_INTERNAL_ERROR,
					EXCEPTION_NONCONTINUABLE,0,NULL);
		#endif
	#else
		#ifdef _PLATFORM_MACINTOSH_
			RAISE(abortErr);
		#else
			abort();
		#endif
	#endif
#endif
}

SOMEXTERN void SOMLINK somAssert(int condition,int ecode,const char *fileName,int lineNum,const char *msg)
{
	if (!condition)
	{
		somPrintf("somAssert(ecode=%d,fileName=%s,lineNum=%d,msg=%s)\n",
				ecode,fileName,lineNum,msg);
		somAssertFailed();
	}
}

SOMEXTERN char * SOMLINK somExceptionId(Environment *ev)
{
	if (ev)
	{
		return ev->exception._exception_name;
	}

	return NULL;
}

SOMEXTERN void * SOMLINK somExceptionValue(Environment *ev)
{
	if (ev)
	{
		return ev->exception._params;
	}

	return NULL;
}

SOMEXTERN void SOMLINK somExceptionFree(Environment *ev)
{
	if (ev)
	{
		if (NO_EXCEPTION != ev->_major)
		{
			if (ev->exception._exception_name)
			{
				SOMFree(ev->exception._exception_name);
			}
			if (ev->exception._params)
			{
				SOMFree(ev->exception._params);
			}

			ev->_major=NO_EXCEPTION;
		}

		ev->exception._exception_name=NULL;
		ev->exception._params=NULL;
	}
}

SOMEXTERN void SOMLINK somSetException(Environment *ev,
    exception_type major,const char *exception_name, void *params)
{
	RHBOPT_ASSERT(ev)

	if (ev)
	{
		RHBOPT_ASSERT(NO_EXCEPTION==ev->_major);
		RHBOPT_ASSERT(NO_EXCEPTION!=major);
		RHBOPT_ASSERT(exception_name);
		RHBOPT_ASSERT(!memcmp(exception_name,"::",2));

		ev->_major=major;

		if (exception_name)
		{
			int i=(int)strlen(exception_name)+1;
			ev->exception._exception_name=SOMMalloc(i);
			if (ev->exception._exception_name)
			{
				memcpy(ev->exception._exception_name,exception_name,i);
			}
		}
		else
		{
			ev->exception._exception_name=NULL;
		}

		ev->exception._params=params;
	}
}
