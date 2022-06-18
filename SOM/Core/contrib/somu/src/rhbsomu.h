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

#define SOMMProxyForObject_VA_EXTERN

#define SOM_Module_snglicls_Source
#define SOM_Module_somproxy_Source
#define SOM_Module_somida_Source
#define SOM_Module_somsid_Source

#ifdef _WIN32
	void SOMTHRD_init(void);
	void SOMTHRD_term(void);
	void SOMTHRD_thread_dead(void);
#endif

#include <som.h>

#ifdef SOMDLLEXPORT 
	#define SOM_IMPORTEXPORT_somu SOMDLLEXPORT
#else
	#define SOM_IMPORTEXPORT_somu
#endif
