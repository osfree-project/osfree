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

#ifndef somthrd_h
#define somthrd_h

#ifdef __cplusplus
	#include <som.xh>
#else
	#include <som.h>
#endif

#ifndef SOM_IMPORTEXPORT_somu
	#ifdef SOMDLLIMPORT 
		#define SOM_IMPORTEXPORT_somu SOMDLLIMPORT
	#else
		#define SOM_IMPORTEXPORT_somu
	#endif
#endif /* SOM_IMPORTEXPORT_somu */

typedef void SOMLINK somTD_SOMThreadProc(void *data);

typedef unsigned long SOMLINK somTD_SOMStartThread(somToken *thrd,somTD_SOMThreadProc *proc,void * data,unsigned long datasz,unsigned long stacksz);
typedef unsigned long SOMLINK somTD_SOMEndThread(void);
typedef unsigned long SOMLINK somTD_SOMKillThread(somToken thrd);
typedef unsigned long SOMLINK somTD_SOMYieldThread(void);
typedef unsigned long SOMLINK somTD_SOMGetThreadHandle(somToken * thrd);

SOMEXTERN SOM_IMPORTEXPORT_somu somTD_SOMStartThread     * SOMDLINK SOMStartThread;
SOMEXTERN SOM_IMPORTEXPORT_somu somTD_SOMEndThread       * SOMDLINK SOMEndThread;
SOMEXTERN SOM_IMPORTEXPORT_somu somTD_SOMKillThread      * SOMDLINK SOMKillThread;
SOMEXTERN SOM_IMPORTEXPORT_somu somTD_SOMYieldThread     * SOMDLINK SOMYieldThread;
SOMEXTERN SOM_IMPORTEXPORT_somu somTD_SOMGetThreadHandle * SOMDLINK SOMGetThreadHandle;

#endif
