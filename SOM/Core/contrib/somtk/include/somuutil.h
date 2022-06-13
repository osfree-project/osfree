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

 /********************************************************************
 *
 * these provide access the configuration file
 *
 * SOMENV variable is supposed to point to actual 'somenv.ini' file,eg
 * SOMENV=c:\som32\etc\somenv.ini 
 */

#ifndef SOMUUTIL_H
#define SOMUUTIL_H

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

/******************************************************
 * it does not seem to matter whether the stanza is in square brackets or not
 * eg [SOMD_TCPIP],
 * caller should NOT free the returned strings
 */

SOM_IMPORTEXPORT_somu char * SOMLINK somutgetenv(char *envvar, char *stanza);

/************************************************
 *	this will look up the environment variable, 
 *	then attempts to get the var from the file 
 */
SOM_IMPORTEXPORT_somu char * SOMLINK somutgetshellenv(char *envvar, char *stanza);

/*************************************
 * 	this closes currently open file
 *	then sets newenv as new name to open 
 *  if NULL is given it uses the name from SOMENV
 */

SOM_IMPORTEXPORT_somu void SOMLINK somutresetenv(char *newenv);

/*************************************************
 * this returns the 'dsom' path similar to SOMDDIR except
 *	 - SOMDDIR environment does not include trailing '\' 
 *   - function is not based on the SOMDDIR variable
 * if SOMBASE is set then it returns
 *      "%SOMBASE%\etc\dsom\"
 * else it returns
 *      "c:\som\etc\dsom\"
 * zero return indicates success,
 */

SOM_IMPORTEXPORT_somu int SOMLINK somutgetpath(char *path);

/***************************************************
 * these are primarily for Macintosh CFM-[PPC/68k]
 */

#ifdef PRAGMA_IMPORT_SUPPORTED
	#if PRAGMA_IMPORT_SUPPORTED
		#ifdef BUILD_SOMU
			/* these should really be put in the module itself */
			#pragma export list somutgetenv,somutgetshellenv,somutresetenv,somutgetpath
		#else
			#pragma import list somutgetenv,somutgetshellenv,somutresetenv,somutgetpath
		#endif
	#endif
#endif /* PRAGMA_IMPORT_SUPPORTED */

#ifdef __cplusplus
}
#endif

#endif
