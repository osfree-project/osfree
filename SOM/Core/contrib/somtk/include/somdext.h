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

#ifndef somdext_h
#define somdext_h

#ifdef __cplusplus
#include <somd.xh>
#else
#include <somd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef void SOMLINK SOMD_Callback(void *, void *);
#ifdef __cplusplus
}
#endif

#ifndef SOMDEXT_IMPORT
	#ifdef SOMDLLIMPORT
		#define SOMDEXT_IMPORT SOMDLLIMPORT
	#else
		#define SOMDEXT_IMPORT 
	#endif
#endif

#ifndef SOM_IMPORTEXPORT_somd
	#if defined(SOMDLLIMPORT) && !defined(SOM_DLL_somd)
		#define SOM_IMPORTEXPORT_somd SOMDLLIMPORT
	#else
		#define SOM_IMPORTEXPORT_somd
	#endif
#endif /* SOM_IMPORTEXPORT_somd */

SOMEXTERN 
#if defined(SOM_DLL_somdcomm)
	#ifdef SOMDLLEXPORT
		SOMDLLEXPORT
	#endif
#else
	#ifdef SOMDLLIMPORT
		SOMDLLIMPORT
	#endif
#endif
void SOMLINK SOMD_FreeType(Environment *,void *,TypeCode);

SOMEXTERN SOMDEXT_IMPORT void SOMLINK SOMD_Init(Environment *);
SOMEXTERN SOMDEXT_IMPORT void SOMLINK SOMD_Uninit(Environment *);
SOMEXTERN SOMDEXT_IMPORT void SOMLINK SOMD_RegisterCallback(SOMObject SOMSTAR emanObj, SOMD_Callback *func);
SOMEXTERN SOMDEXT_IMPORT void SOMLINK somdExceptionFree(Environment *ev);
SOMEXTERN SOMDEXT_IMPORT SOMClass SOMSTAR SOMLINK somdCreateDynProxyClass(Environment *ev,char * targetClassName,long classMajorVersion,long classMinorVersion);
SOMEXTERN SOMDEXT_IMPORT void SOMLINK SOMD_NoORBfree(void);
SOMEXTERN SOMDEXT_IMPORT unsigned char SOMLINK SOMD_QueryORBfree(void);
SOMEXTERN SOMDEXT_IMPORT void SOMLINK SOMD_FlushInterfaceCache(Environment *ev,char * name);
SOMEXTERN SOMDEXT_IMPORT void SOMLINK ORBfree(void *pv);
SOMEXTERN SOMDEXT_IMPORT ReferenceData SOMLINK somdGetDefaultObjectKey(Environment * ev,char * ImplId);
SOMEXTERN SOMDEXT_IMPORT boolean SOMLINK somdDaemonReady(Environment *ev,long timeout);
SOMEXTERN SOMDEXT_IMPORT boolean SOMLINK somdDaemonRequired(Environment *ev);
SOMEXTERN SOMDEXT_IMPORT int SOMLINK somdStartProcess(Environment *ev,char *path,char *name,char *implid,unsigned long winSvrObj,char * title);
SOMEXTERN SOMDEXT_IMPORT SOMObject SOMSTAR SOMLINK somdCreate(Environment *ev,char *clsName,boolean doInit);
SOMEXTERN SOMDEXT_IMPORT ImplId SOMLINK somdExtractUUID(Environment * ev,ReferenceData * id);

#ifdef SOM_RESOLVE_DATA
	SOMEXTERN SOMDEXT_IMPORT ORB				SOMSTAR * SOMLINK resolve_SOMD_ORBObject(void);
	SOMEXTERN SOMDEXT_IMPORT SOMDServer			SOMSTAR * SOMLINK resolve_SOMD_ServerObject(void);
	SOMEXTERN SOMDEXT_IMPORT SOMDObjectMgr		SOMSTAR * SOMLINK resolve_SOMD_ObjectMgr(void);
	SOMEXTERN SOMDEXT_IMPORT SOMOA				SOMSTAR * SOMLINK resolve_SOMD_SOMOAObject(void);
	SOMEXTERN SOMDEXT_IMPORT ImplementationDef	SOMSTAR * SOMLINK resolve_SOMD_ImplDefObject(void);
	SOMEXTERN SOMDEXT_IMPORT ImplRepository		SOMSTAR * SOMLINK resolve_SOMD_ImplRepObject(void);
	SOMEXTERN SOMDEXT_IMPORT Context			SOMSTAR * SOMLINK resolve_SOMD_DefaultContext(void);
	SOMEXTERN SOMDEXT_IMPORT long					    * SOMLINK resolve_SOMD_TraceLevel(void);
	SOMEXTERN SOMDEXT_IMPORT long						* SOMLINK resolve_SOMD_DebugFlag(void);
	SOMEXTERN SOMDEXT_IMPORT long						* SOMLINK resolve_SOMD_RecvBufferSize(void);

	#if defined(SOM_DLL_somd) && !defined(RHBOPT_SHARED_DATA)
		#define SOMD_ORBObject		_SOMD_ORBObject
		#define SOMD_ServerObject	_SOMD_ServerObject
		#define SOMD_ObjectMgr		_SOMD_ObjectMgr
		#define SOMD_SOMOAObject	_SOMD_SOMOAObject
		#define SOMD_ImplDefObject	_SOMD_ImplDefObject
		#define SOMD_ImplRepObject	_SOMD_ImplRepObject
		#define SOMD_DefaultContext	_SOMD_DefaultContext
		#define SOMD_TraceLevel		_SOMD_TraceLevel
		#define SOMD_DebugFlag		_SOMD_DebugFlag
		#define SOMD_RecvBufferSize	_SOMD_RecvBufferSize
	#else
		#define SOMD_ORBObject		(*(resolve_SOMD_ORBObject()))
		#define SOMD_ServerObject	(*(resolve_SOMD_ServerObject()))
		#define SOMD_ObjectMgr		(*(resolve_SOMD_ObjectMgr()))
		#define SOMD_SOMOAObject	(*(resolve_SOMD_SOMOAObject()))
		#define SOMD_ImplDefObject	(*(resolve_SOMD_ImplDefObject()))
		#define SOMD_ImplRepObject	(*(resolve_SOMD_ImplRepObject()))
		#define SOMD_DefaultContext	(*(resolve_SOMD_DefaultContext()))
		#define SOMD_TraceLevel		(*(resolve_SOMD_TraceLevel()))
		#define SOMD_DebugFlag		(*(resolve_SOMD_DebugFlag()))
		#define SOMD_RecvBufferSize	(*(resolve_SOMD_RecvBufferSize()))
	#endif
#endif

#if (!defined(SOM_RESOLVE_DATA)) || (defined(SOM_DLL_somd) && !defined(RHBOPT_SHARED_DATA))
	SOMEXTERN SOM_IMPORTEXPORT_somd ORB					SOMSTAR SOMDLINK SOMD_ORBObject;
	SOMEXTERN SOM_IMPORTEXPORT_somd SOMDServer			SOMSTAR SOMDLINK SOMD_ServerObject;
	SOMEXTERN SOM_IMPORTEXPORT_somd SOMDObjectMgr		SOMSTAR SOMDLINK SOMD_ObjectMgr;
	SOMEXTERN SOM_IMPORTEXPORT_somd SOMOA  				SOMSTAR SOMDLINK SOMD_SOMOAObject;
	SOMEXTERN SOM_IMPORTEXPORT_somd ImplementationDef	SOMSTAR SOMDLINK SOMD_ImplDefObject;
	SOMEXTERN SOM_IMPORTEXPORT_somd ImplRepository		SOMSTAR SOMDLINK SOMD_ImplRepObject;
	SOMEXTERN SOM_IMPORTEXPORT_somd Context				SOMSTAR SOMDLINK SOMD_DefaultContext;
	SOMEXTERN SOM_IMPORTEXPORT_somd long					    SOMDLINK SOMD_TraceLevel; 
	SOMEXTERN SOM_IMPORTEXPORT_somd long						SOMDLINK SOMD_DebugFlag;
	SOMEXTERN SOM_IMPORTEXPORT_somd long						SOMDLINK SOMD_RecvBufferSize;
#endif

#ifdef PRAGMA_IMPORT_SUPPORTED
	#if PRAGMA_IMPORT_SUPPORTED
		#ifdef SOM_DLL_somd
			#pragma export list SOMD_Init,SOMD_Uninit,SOMD_ORBObject,SOMD_ServerObject, \
					SOMD_ObjectMgr,SOMD_SOMOAObject,SOMD_ImplDefObject,SOMD_ImplRepObject, \
					somdExceptionFree,SOMD_RegisterCallback,\
					somdGetDefaultObjectKey,send_multiple_requests,get_next_response,\
					SOMD_DefaultContext,somdCreateDynProxyClass,SOMD_NoORBfree,\
					SOMD_QueryORBfree,SOMD_FlushInterfaceCache,ORBfree,\
					somdDaemonReady,somdDaemonRequired,somdCreate
		#else
			#pragma import list SOMD_Init,SOMD_Uninit,SOMD_ORBObject,SOMD_ServerObject, \
					SOMD_ObjectMgr,SOMD_SOMOAObject,SOMD_ImplDefObject,SOMD_ImplRepObject, \
					somdExceptionFree,SOMD_RegisterCallback,\
					somdGetDefaultObjectKey,send_multiple_requests,get_next_response,\
					SOMD_DefaultContext,somdCreateDynProxyClass,SOMD_NoORBfree,\
					SOMD_QueryORBfree,SOMD_FlushInterfaceCache,ORBfree,\
					somdDaemonReady,somdDaemonRequired,somdCreate
		#endif
		#ifdef SOM_DLL_somdcomm
			#pragma export list SOMD_FreeType
		#else
			#pragma import list SOMD_FreeType
		#endif
	#endif /* PRAGMA_IMPORT_SUPPORTED */
#endif /* PRAGMA_IMPORT_SUPPORTED */

#endif /* somdext_h */

