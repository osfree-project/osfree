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
#include <stdio.h>
#include <som.h>
#include <containr.h>
#include <containd.h>
#include <repostry.h>

#ifdef _PLATFORM_MACINTOSH_
#	include <rhbpkern.h>

static void my_callback(rhbpkern_thread_struct *thr,void *refCon,long delay);

static int SOMLINK my_output(char c)
{
	if (c) printf("%c",c);
	return 1;
}
#endif


int main(int argc,char **argv)
{
	SOMClassMgr SOMSTAR mgr=NULL;
	SOMObject SOMSTAR o=NULL;
	int retVal=1;

#ifdef _PLATFORM_MACINTOSH_
	InitCursorCtl(0);
	rhbpkern_set_callback(my_callback,0);
#endif

#ifdef USE_APPLE_SOM
	somEnvironmentNew();
	mgr=somGetClassManagerReference();
	somSetOutChar(my_output);
#else
	mgr=somEnvironmentNew();
#endif



	/* few problems with NetBSD not having a recursive ld.so */
#if defined(BUILD_STATIC)
	RepositoryNewClass(Repository_MajorVersion,Repository_MinorVersion);
#endif

	o=SOMClassMgr__get_somInterfaceRepository(mgr);

	if (!o)
	{
		somId id=somIdFromString("Repository");
		SOMClass SOMSTAR cls=SOMClassMgr_somFindClass(mgr,id,0,0);

		if (cls)
		{
			o=SOMClass_somNew(cls);
#ifdef USE_APPLE_SOM
			somReleaseClassReference(cls);
#endif
		}
		else
		{
			somPrintf("%s: failed to load \"Repository\" class\n",argv[0]);
		}

		SOMFree(id);
	}

	if (o)
	{
		retVal=0;

		SOMObject_somPrintSelf(o);

		if (argc > 1)
		{
			int i=1;
			somId id=somIdFromString("lookup_id");
			
			while ((i < argc) && (!retVal))
			{
				SOMObject SOMSTAR o2=NULL;
				Environment ev;
				SOM_InitEnvironment(&ev);

				somva_SOMObject_somDispatch(o,
							(somToken *)(void *)&o2,
							id,
							o,&ev,argv[i]);

				if (o2)
				{
					SOMObject_somDumpSelf(o2,0);

#ifdef USE_APPLE_SOM
					somReleaseObjectReference(o2);
#else
					SOMObject_somFree(o2);
#endif
				}
				else
				{
					retVal=1;

					if (ev._major)
					{
						somPrintf("%s\n",somExceptionId(&ev));
					}
				}

				i++;

				SOM_UninitEnvironment(&ev);
			}

			SOMFree(id);
		}
		else
		{
			SOMObject_somDumpSelf(o,0);
		}

#ifdef USE_APPLE_SOM
		somReleaseObjectReference(o);
#else
		SOMObject_somFree(o);
#endif
	}
	else
	{
		somPrintf("%s: failed to get repository object\n",argv[0]);
	}

#ifdef USE_APPLE_SOM
	somReleaseClassManagerReference(mgr);
#endif

	somEnvironmentEnd();

	return retVal;
}

#ifdef _PLATFORM_MACINTOSH_
void my_callback(rhbpkern_thread_struct *thr,void *refCon,long delay)
{
	SpinCursor(1);
}
#endif
