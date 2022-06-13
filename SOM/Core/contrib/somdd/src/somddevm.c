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

#ifdef _WIN32
	#define SOMDLLEXPORT
#endif

#include <rhbopt.h>
#include <rhbsomdd.h>

#ifndef USE_THREADS

	#define SOMDDEventMgr_Class_Source

	#include <somddevm.ih>

	SOM_Scope void SOMLINK somddeman_somUninit(SOMDDEventMgr SOMSTAR somSelf)
	{
		SOMDDEventMgr_parent_SOMEEMan_somUninit(somSelf);
	}

	SOM_Scope void SOMLINK somddeman_somInit(SOMDDEventMgr SOMSTAR somSelf)
	{
		SOMDDEventMgr_parent_SOMEEMan_somInit(somSelf);
	}

	#ifdef SOMEEMan_someWait
	SOM_Scope void SOMLINK somddeman_someWait(SOMDDEventMgr SOMSTAR somSelf,Environment *ev,unsigned long delay)
	{
	#ifdef _PLATFORM_MACINTOSH_
		SOMDD_someWait(delay);
	#else
		SOMDDEventMgr_parent_SOMEEMan_someWait(somSelf,ev,delay);
	#endif
	}
	#endif

#endif
