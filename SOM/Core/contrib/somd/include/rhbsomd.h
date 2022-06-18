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

#ifdef _WIN32
	#include <process.h>

	#ifdef HAVE_WINSOCK2_H
		#include <winsock2.h>
	#else
		#include <winsock.h>
	#endif

	#ifdef HAVE_WS2IP6_H
		#include <ws2ip6.h>
	#endif

	#ifdef HAVE_WS2TCPIP_H
		#include <ws2tcpip.h>
	#endif

	const char * __stdcall
		inet_ntop(int af, const void *src, char *dst, int size);

	int __stdcall inet_pton(int af, const char *src, void *dst);
#endif

#ifdef _DEBUG
	#define RHBSOMD_debug_threads
#endif

/* to get around some magic... */

#include <rhbsomex.h>
#include <rhbendia.h>
#include <somtc.h>
#include <somtcnst.h>

#define SOM_DLL_somd

#ifdef SOMDLLEXPORT
	#define SOM_IMPORTEXPORT_somd   SOMDLLEXPORT
	#define SOMDEXT_IMPORT			SOMDLLEXPORT
#else
	#define SOM_IMPORTEXPORT_somd  
#endif

#ifndef SOM_Scope
	#define SOM_Scope		static
#endif

/* these macros are because all of these files are in the whole
	module, it is better for various globals */
#define SOM_Module_principl_Source
#define SOM_Module_somdobj_Source
#define SOM_Module_orb_Source
#define SOM_Module_somoa_Source
#define SOM_Module_boa_Source
#define SOM_Module_poa_Source
#define SOM_Module_request_Source
#define SOM_Module_somdcprx_Source
#define SOM_Module_somdmprx_Source
#define SOM_Module_nvlist_Source
#define SOM_Module_cntxt_Source
#define SOM_Module_om_Source
#define SOM_Module_impldef_Source
#define SOM_Module_impldefp_Source
#define SOM_Module_implrep_Source
#define SOM_Module_implrepp_Source
#define SOM_Module_servmgr_Source
#define SOM_Module_somdom_Source
#define SOM_Module_somdserv_Source
#define SOM_Module_somdtype_Source
#define SOM_Module_rhborbst_Source
#define SOM_Module_unotypes_Source
#define SOM_Module_servreq_Source
#define SOM_Module_somdfenc_Source
#define SOM_Module_somdstub_Source

#ifndef ImplRepository_Class_Source
	#define ImplRepository_VA_EXTERN
#endif

#ifndef SOMDServerMgr_Class_Source
	#define SOMDServerMgr_VA_EXTERN
#endif

typedef struct RHBORB RHBORB;
typedef struct RHBGIOPRequestStream RHBGIOPRequestStream;
typedef struct RHBServerRequest RHBServerRequest;
typedef struct RHBServerRequest RHBLocateRequest;
typedef struct RHBObject RHBObject;
typedef struct RHBRequest RHBRequest;

#include <somref.h>
#include <containd.h>
#include <containr.h>
#include <rhbsomkd.h>
#include <rhbsomut.h>
#include <rhbmtut.h>
#include <somd.h>
#include <somdtype.h>
#include <paramdef.h>
#include <excptdef.h>
#include <operatdf.h>
#include <attribdf.h>
#include <intfacdf.h>
#include <repostry.h>

#define RHBProcessInfo  somToken

#include <somdmprx.h>
#include <somdext.h>
#include <somderr.h>
#include <rhbgiop.h>

#ifdef Contained
/* this is Apple SOM, it uses defines badly for object types */
	#ifndef CORBA_InterfaceDef
		#define CORBA_InterfaceDef  SOMObject
	#endif
	#ifndef CORBA_Contained
		#define CORBA_Contained  SOMObject
	#endif
	#ifndef CORBA_IDLType
		#define CORBA_IDLType  SOMObject
	#endif
#else
	/* this is not apple SOM */
#endif

#include <rhborbst.h>
#include <corba.h>
#include <poa.h>
#include <servreq.h>
#include <somdfenc.h>
#include <somdstub.h>

/* this stuff stops linking with SOMIR */

#ifdef Repository_lookup_id
	#undef Repository_lookup_id
#endif

#define Repository_lookup_id     use ORB_lookup_id

#ifdef Contained_describe
	#undef Contained_describe
#endif
#define Contained_describe      use ORB_contained_describe

#ifdef Contained__get_somModifiers
	#undef Contained__get_somModifiers
#endif
#define Contained__get_somModifiers      use ORB_contained_describe

#ifdef Contained__get_id
	#undef Contained__get_id
#endif

#ifdef Contained__get_absolute_name
	#undef Contained__get_absolute_name
#endif

#define Contained__get_absolute_name  use ORB_contained_describe

SOMEXTERN void SOMLINK SOMInitsomd(void);

#define x_printf		somPrintf 

#ifndef x_printf
	#define x_printf   printf
	#define somPrintf  do not use this fn directly
#endif

#define dump_somPrintf(x)		x_printf x

#ifdef _DEBUG_x
	#define debug_somPrintf(print_params)      x_printf print_params
#else
	#define debug_somPrintf(x)   
#endif

#ifdef _PLATFORM_MACINTOSH_
#define SOMD_bomb(x)  { Str255 str; str[0]=sprintf((char *)&str[1],"%s:%d, %s",__FILE__,__LINE__,x); DebugStr(str); }
#else
	#define SOMD_bomb(x)       {  x_printf("%s",x); SOMError(-1,__FILE__,__LINE__); }
#endif

#ifndef SOMClass_somDuplicateReference
	#define SOMClass_somDuplicateReference(x)      x
#endif

#define OBJECTKEY_PREFIX_LEN    (2+ImplementationDef_SOMD_MAXLEN_IMPL_ID)

#ifndef SOMD_TypeCodes
#define SOMD_TypeCodes

extern somToken somdTC__GIOP_RequestHeader_1_0,
			    somdTC__GIOP_MessageHeader,
				somdTC__GIOP_ReplyHeader_1_0,
				somdTC__GIOP_LocateRequestHeader,
				somdTC__GIOP_LocateReplyHeader_1_0,
				somdTC__GIOP_CancelRequestHeader,
				somdTC__IOP_IOR,
				somdTC__IOP_TaggedComponent,
				somdTC__IOP_TaggedProfile,
				somdTC__IIOP_ProfileBody_1_0,
				somdTC__IIOP_ProfileBody_1_1,
				somdTC__sequence_octet,
				somdTC__Contained_Description,
				somdTC__IRObject_DefinitionKind,
				somdTC__sequence_somModifier,
				somdTC__sequence_SOMObject,
				somdTC__sequence_SOMD_NetworkAddress,
				somdTC__StExcep,
				somdTC__CORBA_InterfaceDef,
				somdTC__IRObject_DefinitionKind,
				somdTC__SOMDServer;

#define somdTC_GIOP_RequestHeader_1_0		((TypeCode)(void *)&somdTC__GIOP_RequestHeader_1_0)
#define somdTC_GIOP_MessageHeader			((TypeCode)(void *)&somdTC__GIOP_MessageHeader)
#define somdTC_GIOP_ReplyHeader_1_0			((TypeCode)(void *)&somdTC__GIOP_ReplyHeader_1_0)
#define somdTC_GIOP_LocateRequestHeader		((TypeCode)(void *)&somdTC__GIOP_LocateRequestHeader)
#define somdTC_GIOP_LocateReplyHeader_1_0	((TypeCode)(void *)&somdTC__GIOP_LocateReplyHeader_1_0)
#define somdTC_GIOP_CancelRequestHeader		((TypeCode)(void *)&somdTC__GIOP_CancelRequestHeader)
#define somdTC_IOP_IOR						((TypeCode)(void *)&somdTC__IOP_IOR)
#define somdTC_IOP_TaggedComponent			((TypeCode)(void *)&somdTC__IOP_TaggedComponent)
#define somdTC_IOP_TaggedProfile			((TypeCode)(void *)&somdTC__IOP_TaggedProfile)
#define somdTC_IIOP_ProfileBody_1_0			((TypeCode)(void *)&somdTC__IIOP_ProfileBody_1_0)
#define somdTC_IIOP_ProfileBody_1_1			((TypeCode)(void *)&somdTC__IIOP_ProfileBody_1_1)
#define somdTC_sequence_octet				((TypeCode)(void *)&somdTC__sequence_octet)
#define somdTC_Contained_Description		((TypeCode)(void *)&somdTC__Contained_Description)
#define somdTC_IRObject_DefinitionKind		((TypeCode)(void *)&somdTC__IRObject_DefinitionKind)
#define somdTC_sequence_somModifier			((TypeCode)(void *)&somdTC__sequence_somModifier)
#define somdTC_StExcep						((TypeCode)(void *)&somdTC__StExcep)
#define somdTC_sequence_SOMObject			((TypeCode)(void *)&somdTC__sequence_SOMObject)
#define somdTC_sequence_SOMD_NetworkAddress	((TypeCode)(void *)&somdTC__sequence_SOMD_NetworkAddress)
#define somdTC_CORBA_InterfaceDef			((TypeCode)(void *)&somdTC__CORBA_InterfaceDef)
#define somdTC_SOMDServer					((TypeCode)(void *)&somdTC__SOMDServer)

#define RHBSOMD_Init_NamedValue  {NULL,{NULL,NULL},0,0}

#define somExceptionFree(x)			somdExceptionFree(x)

#undef SOM_UninitEnvironment
#define SOM_UninitEnvironment(x)	somdExceptionFree(x)

#ifdef HAVE_SNPRINTF
#	ifdef sprintf
#		undef sprintf
#	endif
#	define sprintf  do not use
#endif

/* the SOMRefObject'ness of some objects is lost */

#ifndef ImplementationDef_somDuplicateReference
#define ImplementationDef_somDuplicateReference(x) SOMRefObject_somDuplicateReference(x)
#endif

#ifndef CORBA_DataInputStream_somDuplicateReference
#define CORBA_DataInputStream_somDuplicateReference(x)  SOMRefObject_somDuplicateReference(x)
#endif

#ifndef RHBORBStream_GIOPInput_somRelease
#define RHBORBStream_GIOPInput_somRelease(x)  SOMRefObject_somRelease(x)
#endif

#ifndef SOMEEMRegisterData_somRelease
#define SOMEEMRegisterData_somRelease(x)	  SOMRefObject_somRelease(x)
#endif


#endif
