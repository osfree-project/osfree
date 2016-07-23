/* generated from request.idl */
/* internal conditional is SOM_Module_request_Source */
#ifndef SOM_Module_request_Header_h
	#define SOM_Module_request_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#ifndef _IDL_Request_defined
		#define _IDL_Request_defined
		typedef SOMObject Request;
	#endif /* _IDL_Request_defined */
	#ifndef _IDL_SEQUENCE_Request_defined
		#define _IDL_SEQUENCE_Request_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Request SOMSTAR ,sequence(Request));
	#endif /* _IDL_SEQUENCE_Request_defined */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Request_add_arg)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier name,
			/* in */ TypeCode arg_type,
			/* in */ void *value,
			/* in */ long len,
			/* in */ Flags arg_flags);
		#pragma linkage(somTP_Request_add_arg,system)
		typedef somTP_Request_add_arg *somTD_Request_add_arg;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Request_add_arg)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier name,
			/* in */ TypeCode arg_type,
			/* in */ void *value,
			/* in */ long len,
			/* in */ Flags arg_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Request_invoke)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags invoke_flags);
		#pragma linkage(somTP_Request_invoke,system)
		typedef somTP_Request_invoke *somTD_Request_invoke;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Request_invoke)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags invoke_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Request_send)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags invoke_flags);
		#pragma linkage(somTP_Request_send,system)
		typedef somTP_Request_send *somTD_Request_send;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Request_send)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags invoke_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Request_get_response)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags response_flags);
		#pragma linkage(somTP_Request_get_response,system)
		typedef somTP_Request_get_response *somTD_Request_get_response;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Request_get_response)(
			Request SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags response_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Request_destroy)(
			Request SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Request_destroy,system)
		typedef somTP_Request_destroy *somTD_Request_destroy;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Request_destroy)(
			Request SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef Request_MajorVersion
		#define Request_MajorVersion   2
	#endif /* Request_MajorVersion */
	#ifndef Request_MinorVersion
		#define Request_MinorVersion   2
	#endif /* Request_MinorVersion */
	typedef struct RequestClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken add_arg;
		somMToken invoke;
		somMToken send;
		somMToken get_response;
		somMToken destroy;
		somMToken _get_c_request;
		somMToken _set_c_request;
	} RequestClassDataStructure;
	typedef struct RequestCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} RequestCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_request_Source) || defined(Request_Class_Source)
			SOMEXTERN struct RequestClassDataStructure _RequestClassData;
			#ifndef RequestClassData
				#define RequestClassData    _RequestClassData
			#endif /* RequestClassData */
		#else
			SOMEXTERN struct RequestClassDataStructure * SOMLINK resolve_RequestClassData(void);
			#ifndef RequestClassData
				#define RequestClassData    (*(resolve_RequestClassData()))
			#endif /* RequestClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_request_Source) || defined(Request_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_request_Source || Request_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_request_Source || Request_Class_Source */
		struct RequestClassDataStructure SOMDLINK RequestClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_request_Source) || defined(Request_Class_Source)
			SOMEXTERN struct RequestCClassDataStructure _RequestCClassData;
			#ifndef RequestCClassData
				#define RequestCClassData    _RequestCClassData
			#endif /* RequestCClassData */
		#else
			SOMEXTERN struct RequestCClassDataStructure * SOMLINK resolve_RequestCClassData(void);
			#ifndef RequestCClassData
				#define RequestCClassData    (*(resolve_RequestCClassData()))
			#endif /* RequestCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_request_Source) || defined(Request_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_request_Source || Request_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_request_Source || Request_Class_Source */
		struct RequestCClassDataStructure SOMDLINK RequestCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_request_Source) || defined(Request_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_request_Source || Request_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_request_Source || Request_Class_Source */
	SOMClass SOMSTAR SOMLINK RequestNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_Request (RequestClassData.classObject)
	#ifndef SOMGD_Request
		#if (defined(_Request) || defined(__Request))
			#undef _Request
			#undef __Request
			#define SOMGD_Request 1
		#else
			#define _Request _SOMCLASS_Request
		#endif /* _Request */
	#endif /* SOMGD_Request */
	#define Request_classObj _SOMCLASS_Request
	#define _SOMMTOKEN_Request(method) ((somMToken)(RequestClassData.method))
	#ifndef RequestNew
		#define RequestNew() ( _Request ? \
			(SOMClass_somNew(_Request)) : \
			( RequestNewClass( \
				Request_MajorVersion, \
				Request_MinorVersion), \
			SOMClass_somNew(_Request))) 
	#endif /* NewRequest */
	#ifndef Request_add_arg
		#define Request_add_arg(somSelf,ev,name,arg_type,value,len,arg_flags) \
			SOM_Resolve(somSelf,Request,add_arg)  \
				(somSelf,ev,name,arg_type,value,len,arg_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__add_arg
				#if defined(_add_arg)
					#undef _add_arg
					#define SOMGD__add_arg
				#else
					#define _add_arg Request_add_arg
				#endif
			#endif /* SOMGD__add_arg */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Request_add_arg */
	#ifndef Request_invoke
		#define Request_invoke(somSelf,ev,invoke_flags) \
			SOM_Resolve(somSelf,Request,invoke)  \
				(somSelf,ev,invoke_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__invoke
				#if defined(_invoke)
					#undef _invoke
					#define SOMGD__invoke
				#else
					#define _invoke Request_invoke
				#endif
			#endif /* SOMGD__invoke */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Request_invoke */
	#ifndef Request_send
		#define Request_send(somSelf,ev,invoke_flags) \
			SOM_Resolve(somSelf,Request,send)  \
				(somSelf,ev,invoke_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__send
				#if defined(_send)
					#undef _send
					#define SOMGD__send
				#else
					#define _send Request_send
				#endif
			#endif /* SOMGD__send */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Request_send */
	#ifndef Request_get_response
		#define Request_get_response(somSelf,ev,response_flags) \
			SOM_Resolve(somSelf,Request,get_response)  \
				(somSelf,ev,response_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_response
				#if defined(_get_response)
					#undef _get_response
					#define SOMGD__get_response
				#else
					#define _get_response Request_get_response
				#endif
			#endif /* SOMGD__get_response */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Request_get_response */
	#ifndef Request_destroy
		#define Request_destroy(somSelf,ev) \
			SOM_Resolve(somSelf,Request,destroy)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__destroy
				#if defined(_destroy)
					#undef _destroy
					#define SOMGD__destroy
				#else
					#define _destroy Request_destroy
				#endif
			#endif /* SOMGD__destroy */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Request_destroy */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Request_somInit SOMObject_somInit
#define Request_somUninit SOMObject_somUninit
#define Request_somFree SOMObject_somFree
#define Request_somGetClass SOMObject_somGetClass
#define Request_somGetClassName SOMObject_somGetClassName
#define Request_somGetSize SOMObject_somGetSize
#define Request_somIsA SOMObject_somIsA
#define Request_somIsInstanceOf SOMObject_somIsInstanceOf
#define Request_somRespondsTo SOMObject_somRespondsTo
#define Request_somDispatch SOMObject_somDispatch
#define Request_somClassDispatch SOMObject_somClassDispatch
#define Request_somCastObj SOMObject_somCastObj
#define Request_somResetObj SOMObject_somResetObj
#define Request_somPrintSelf SOMObject_somPrintSelf
#define Request_somDumpSelf SOMObject_somDumpSelf
#define Request_somDumpSelfInt SOMObject_somDumpSelfInt
#define Request_somDefaultInit SOMObject_somDefaultInit
#define Request_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Request_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Request_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Request_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Request_somDefaultAssign SOMObject_somDefaultAssign
#define Request_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Request_somDefaultVAssign SOMObject_somDefaultVAssign
#define Request_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Request_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#define ARG_IN				0x00000001
#define ARG_OUT			0x00000002
#define ARG_INOUT			0x00000004
#define IN_COPY_VALUE		0x00000008
#define DEPENDENT_LIST		0x00000010
#define OUT_LIST_MEMORY	0x00000020
#define INV_NO_RESPONSE	0x00000040
#define INV_TERM_ON_ERR	0x00000080
#define RESP_NO_WAIT		0x00000100
#define AUTH_IN_MSG		0x00010000
#define ACKONEWAY			0x00020000
#define ENV_IN_MSG			0x00040000
#define CTX_IN_MSG			0x00080000
#define OBJ_OWNED			0x00100000
#define CALLER_OWNED		0x00200000
#define RESULT_OUT			0x00400000
#define CLIENT_SIDE		0x00800000
#define SERVER_SIDE		0x01000000
#define TXCTX_IN_MSG		0x02000000
#define ARG_IS_PTRARG		0x10000000
#define ARG_IS_COMPONENT	0x20000000
#define ROOT_NOT_PTRARG	0x40000000
#define RESERVED_FLAGS		0x8c00fe00

#if defined(__IBMC__) && defined(_OS2)
  #pragma linkage(send_multiple_requests, system)
  #pragma linkage(get_next_response, system)
#endif
#ifndef SOM_IMPORTEXPORT_somd
		#ifdef SOMDLLIMPORT
			#define SOM_IMPORTEXPORT_somd SOMDLLIMPORT
		#else
			#define SOM_IMPORTEXPORT_somd
		#endif
#endif
SOMEXTERN SOM_IMPORTEXPORT_somd ORBStatus SOMLINK send_multiple_requests(Request SOMSTAR [],Environment *,long,Flags);
SOMEXTERN SOM_IMPORTEXPORT_somd ORBStatus SOMLINK get_next_response(Environment *,Flags,Request SOMSTAR *req);
#ifndef Request_delete
#define Request_delete Request_destroy
#endif

#endif /* SOM_Module_request_Header_h */
