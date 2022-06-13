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

#ifndef SOMSTUBS_H
#define SOMSTUBS_H

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif
#include <som.h>
#include <stdio.h>

#ifdef SOM_RESOLVE_DATA
	#define SOMSTUB(x)    \
		x##ClassDataStructure * SOMLINK resolve_##x##ClassData(void) { return NULL; } \
		x##CClassDataStructure * SOMLINK resolve_##x##CClassData(void) { return NULL; } \
		SOMClass SOMSTAR SOMLINK x##NewClass(integer4 x,integer4 y) { \
				printf("SOMStub::NewClass(%s)\n",#x); \
				exit(1); \
				return NULL; }
#else
	#define SOMSTUB(x)    \
		x##ClassDataStructure x##ClassData;		\
		x##CClassDataStructure x##CClassData;	\
		SOMClass SOMSTAR SOMLINK x##NewClass(integer4 x,integer4 y) { \
				printf("SOMStub::NewClass(%s)\n",#x); \
				exit(1); \
				return NULL; }
#endif

#ifdef _WIN32
int CALLBACK DllMain(void *n,unsigned long r,void *d)
{
/*	switch (r)
	{
	case DLL_PROCESS_ATTACH:
		{
			char buf[256];
			if (GetModuleFileName(n,buf,sizeof(buf)))
			{
				MessageBox(GetDesktopWindow(),buf,__FILE__,MB_ICONERROR|MB_OK);
			}
		}
		break;
	}
	return 0;*/
	return 1;
}
#endif

#endif
