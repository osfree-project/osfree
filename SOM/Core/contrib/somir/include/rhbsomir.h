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

#ifdef _PLATFORM_WIN32_
#	include <windows.h>
#endif

#include <rhbsomex.h>

#ifdef SOMDLLEXPORT
#	define SOM_IMPORTEXPORT_somir SOMDLLEXPORT
#else
#	define SOM_IMPORTEXPORT_somir
#endif

#define SOM_Module_operatdf_Source
#define SOM_Module_intfacdf_Source
#define SOM_Module_containd_Source
#define SOM_Module_containr_Source
#define SOM_Module_repostry_Source
#define SOM_Module_excptdef_Source
#define SOM_Module_typedef_Source
#define SOM_Module_moduledf_Source
#define SOM_Module_paramdef_Source
#define SOM_Module_constdef_Source
#define SOM_Module_attribdf_Source
#define SOM_Module_rhbifrdb_Source

typedef somToken Contained_private_get;
typedef struct SOMIR_Repository Contained_private_RepositoryTable;
typedef somToken Contained_private_KeyedData;
typedef struct SOMIR_ContainedData somir_containedData;
typedef struct SOMIR_ContainerData somir_containerData;

#include <somtc.h>
#include <somtcnst.h>
#include <somref.h>
#include <containd.h>
#include <containr.h>
#include <excptdef.h>
#include <attribdf.h>
#include <paramdef.h>
#include <operatdf.h>
#include <intfacdf.h>
#include <typedef.h>
#include <constdef.h>
#include <moduledf.h>
#include <repostry.h>
#include <rhbmtut.h>
#include <stexcep.h>
#include <somirfmt.h>
#include <somderr.h>
#include <rhbsomex.h>

#if defined(_WIN32) || defined(_WIN16) || defined(__OS2__)
	#define kSOMIRseparator			';'
#else
	#define kSOMIRseparator			':'
#endif

typedef struct 
{
	Repository SOMSTAR repositoryObject;
} M_Repository_SingletonHolder;

struct SOMIR_RepositoryVtbl
{
	Contained SOMSTAR (*Wrap)(struct SOMIR_ContainedData *,Environment *);
	short (*TypeFromName)(const char *);
	int (*begin_use)(struct SOMIR_Repository *);
	void (*end_use)(struct SOMIR_Repository *);
};

#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
		extern struct rhbmutex_t somir_crit;
#		define SOMIR_LOCK		RHBMUTEX_LOCK(&somir_crit);
#		define SOMIR_UNLOCK		RHBMUTEX_UNLOCK(&somir_crit);
#		define SOMIR_ASSERT_LOCKED RHBMUTEX_ASSERT_LOCKED(&somir_crit);
#	else
		extern CRITICAL_SECTION somir_crit;
#		define SOMIR_LOCK		EnterCriticalSection(&somir_crit);
#		define SOMIR_UNLOCK		LeaveCriticalSection(&somir_crit);
#		define SOMIR_ASSERT_LOCKED
#	endif
#else
#	define SOMIR_LOCK
#	define SOMIR_UNLOCK
#	define SOMIR_ASSERT_LOCKED
#endif

extern void somir_dll_init(void);
extern void somir_dll_term(void);

#if defined(USE_THREADS) && defined(USE_PTHREADS)
#	define SOMIR_DLL_INIT    somir_dll_init();
#else
#	define SOMIR_DLL_INIT
#endif
