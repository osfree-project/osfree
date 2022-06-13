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

/* #define SOM_Scope  static */

#ifndef SOM_IMPORTEXPORT_somnmf
	#ifdef SOMDLLEXPORT
		#define SOM_IMPORTEXPORT_somnmf SOMDLLEXPORT
	#else
		#define SOM_IMPORTEXPORT_somnmf 
	#endif
#endif /* SOM_IMPORTEXPORT_som */


#include <rhbsomex.h>

#define SOM_Module_naming_Source
#define SOM_Module_xnaming_Source
#define SOM_Module_xnamingf_Source
#define SOM_Module_biter_Source
#define SOM_Module_lname_Source
#define SOM_Module_lnamec_Source

#include <rhbsomkd.h>
/*#include <naming.h>
#include <lnamec.h>
#include <lname.h>
#include <biter.h>
#include <xnaming.h>
*/
#include <somnm.h>
#include <somir.h>
#include <somd.h>
#include <somos.h>
#include <xnamingf.h>
