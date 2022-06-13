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

#ifndef SOMUUUID_H
#define SOMUUUID_H

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

#ifdef __cplusplus
extern "C" {
#endif

SOM_IMPORTEXPORT_somu int SOMLINK somCreateUUID(octet *buf);
SOM_IMPORTEXPORT_somu char * SOMLINK somUUIDToString(octet *id,char *strid);
SOM_IMPORTEXPORT_somu int SOMLINK somCompareUUIDStrings(char *id1,char *id2);
SOM_IMPORTEXPORT_somu int SOMLINK somStringToUUID(char *strid,octet *id);

/***************************************************
 * these are primarily for Macintosh CFM-[PPC/68k]
 */

#ifdef PRAGMA_IMPORT_SUPPORTED
	#if PRAGMA_IMPORT_SUPPORTED
		#ifdef BUILD_SOMU
			/* these should really be put in the module itself */
			#pragma export list somCreateUUID,somUUIDToString,somCompareUUIDStrings,somStringToUUID
		#else
			#pragma import list somCreateUUID,somUUIDToString,somCompareUUIDStrings,somStringToUUID
		#endif
	#endif
#endif /* PRAGMA_IMPORT_SUPPORTED */

#ifdef __cplusplus
}
#endif

#endif
