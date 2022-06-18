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

#ifndef SOMNMEXT_H
#define SOMNMEXT_H

#ifndef SOM_IMPORTEXPORT_somnmf
	#ifdef SOMDLLIMPORT
		#define SOM_IMPORTEXPORT_somnmf SOMDLLIMPORT
	#else
		#define SOM_IMPORTEXPORT_somnmf 
	#endif
#endif /* SOM_IMPORTEXPORT_somnmf */

SOMEXTERN SOM_IMPORTEXPORT_somnmf LName SOMSTAR SOMLINK create_lname(void);
SOMEXTERN SOM_IMPORTEXPORT_somnmf LNameComponent SOMSTAR SOMLINK create_lname_component(void);

#ifdef PRAGMA_IMPORT_SUPPORTED
	#if PRAGMA_IMPORT_SUPPORTED
		#ifdef BUILD_SOMNMF
			#pragma export list create_lname,create_lname_component
		#else
			#pragma import list create_lname,create_lname_component
		#endif
	#endif /* PRAGMA_IMPORT_SUPPORTED */
#endif /* PRAGMA_IMPORT_SUPPORTED */

#endif /* SOMNMEXT_H */
