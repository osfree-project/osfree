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

#ifndef __SOMIR__
#define __SOMIR__

#ifdef __cplusplus
	#include <somtc.xh>
	#include <somtcnst.xh>
#else
	#include <somtc.h>
	#include <somtcnst.h>
#endif

#ifndef somModifier_defined
	#define somModifier_defined
	typedef struct somModifier
	{
		char * name;
		char * value;
	} somModifier;
#endif

#ifndef SOM_IMPORTEXPORT_somir
	#ifdef SOMDLLIMPORT
		#define SOM_IMPORTEXPORT_somir SOMDLLIMPORT
	#else
		#define SOM_IMPORTEXPORT_somir 
	#endif
#endif /* SOM_IMPORTEXPORT_somir */

SOMEXTERN SOM_IMPORTEXPORT_somir char * SOMLINK irGetFileNames(void);
SOMEXTERN SOM_IMPORTEXPORT_somir char * SOMLINK irGetLastFileName(void);

#endif

