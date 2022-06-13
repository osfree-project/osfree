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

#define SOM_Module_somos_Source
#define SOM_Module_somap_Source

#include <rhbsomex.h>
#include <somref.h>

#ifndef SOM_IMPORTEXPORT_somos
	#ifdef SOMDLLEXPORT
		#define SOM_IMPORTEXPORT_somos SOMDLLEXPORT
	#else
		#define SOM_IMPORTEXPORT_somos 
	#endif
#endif /* SOM_IMPORTEXPORT_som */

#ifdef __cplusplus
	#include <somd.xh>
	#include <somos.xh>
	#include <somtdm.xh>
	#include <somap.xh>
	#include <somosutl.xh>
#else
	#include <somd.h>
	#include <somos.h>
	#include <somtdm.h>
	#include <somap.h>
	#include <somosutl.h>
#endif

#ifndef SOMDServer_somDuplicateReference
#define SOMDServer_somDuplicateReference(x)   SOMRefObject_somDuplicateReference(x)
#endif

