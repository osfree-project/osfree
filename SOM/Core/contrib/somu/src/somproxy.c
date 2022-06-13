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
#include <rhbsomex.h>

#ifndef USE_APPLE_SOM
	#include <somref.h>
#endif

#define SOMMProxyForObject_VA_STUBS

#ifndef SOM_Module_somproxy_Source
	#define SOM_Module_somproxy_Source
#endif


#define SOMMProxyForObject_Class_Source
#define SOMMProxyFor_Class_Source

#include <rhbsomu.h>
#include <somuutil.h>
#include <somproxy.ih>

#ifdef USE_APPLE_SOM
	#include <rhbdynac.h>
#endif

SOM_Scope boolean SOMLINK somproxy_sommProxyDispatch(
	SOMMProxyForObject SOMSTAR somSelf,
	/* out */ somToken *returnBufferPointer,
	/* in */ SOMMProxyForObject_sommProxyDispatchInfo *dispatchInfo,
	/* in */ va_list ap)
{
	int pointless=va_arg(ap,int);

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(returnBufferPointer);
	SOM_IgnoreWarning(dispatchInfo);
	SOM_IgnoreWarning(pointless);

	return 0;
}

SOM_Scope SOMClass SOMSTAR SOMLINK somproxyClass_sommMakeProxyClass(
	SOMMProxyFor SOMSTAR somSelf,
	/* in */ SOMClass SOMSTAR targetClass,
	/* in */ corbastring className)
{
#ifdef USE_APPLE_SOM
	return RHBCreateDynamicClass(somSelf,targetClass,className);
#else
	_IDL_SEQUENCE_SOMClass seq={2,2,NULL};
	long l=0;
	SOMClass SOMSTAR newClass=NULL;
	SOMClass SOMSTAR metaProxyClass=NULL;
	long major=0,minor=0;
	SOMClass SOMSTAR seq_buffer[2]={NULL,NULL};

	SOMClass_somGetVersionNumbers(targetClass,&major,&minor);

	metaProxyClass=SOMObject_somGetClass(somSelf);

	newClass=(SOMClass SOMSTAR)SOMClass_somNew(metaProxyClass);

/*	seq._length=2;
	seq._maximum=2;*/
	seq._buffer=seq_buffer;

	seq._buffer[0]=somSelf;
	seq._buffer[1]=targetClass;

	SOMClass_somInitMIClass(
			newClass,
			1, /* only inherit implementation of Proxy */
			className,
			&seq,
			0,0,0,major,minor);

	l=SOMClass_somGetNumMethods(targetClass);

	while (l--)
	{
		somId desc=NULL;
		somId descRet=SOMClass_somGetNthMethodInfo(targetClass,l,&desc);

		if (descRet)
		{
			if (!SOMClass_somGetMethodDescriptor(somSelf,descRet))
			{
				somMethodPtr pfn=SOMClass_somGetRdStub(targetClass,descRet);

				if (pfn)
				{
					SOMClass_somOverrideSMethod(newClass,descRet,pfn);
				}
			}
		}
	}

	SOMClass_somClassReady(newClass);

	return newClass;
#endif
}

