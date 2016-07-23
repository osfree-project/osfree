/* generated from impldef.idl */
/* internal conditional is SOM_Module_impldef_Source */
#ifndef SOM_Module_impldef_Header_h
	#define SOM_Module_impldef_Header_h 1
	#include <som.h>
	#include <somdtype.h>
	#include <somobj.h>
	#include <containd.h>
	#include <omgestio.h>
	#include <naming.h>
	#include <omgidobj.h>
	#ifndef _IDL_SOMDServer_defined
		#define _IDL_SOMDServer_defined
		typedef SOMObject SOMDServer;
	#endif /* _IDL_SOMDServer_defined */
	#ifndef _IDL_SEQUENCE_SOMDServer_defined
		#define _IDL_SEQUENCE_SOMDServer_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDServer SOMSTAR ,sequence(SOMDServer));
	#endif /* _IDL_SEQUENCE_SOMDServer_defined */
	#ifndef _IDL_ImplementationDef_defined
		#define _IDL_ImplementationDef_defined
		typedef SOMObject ImplementationDef;
	#endif /* _IDL_ImplementationDef_defined */
	#ifndef _IDL_SEQUENCE_ImplementationDef_defined
		#define _IDL_SEQUENCE_ImplementationDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ImplementationDef SOMSTAR ,sequence(ImplementationDef));
	#endif /* _IDL_SEQUENCE_ImplementationDef_defined */
#ifndef ImplementationDef_SOMD_MAXLEN_IMPL_ID
	#define ImplementationDef_SOMD_MAXLEN_IMPL_ID   36L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SOMD_MAXLEN_IMPL_ID
			#if defined(SOMD_MAXLEN_IMPL_ID)
				#undef SOMD_MAXLEN_IMPL_ID
				#define SOMGD_SOMD_MAXLEN_IMPL_ID
			#else
				#define SOMD_MAXLEN_IMPL_ID ImplementationDef_SOMD_MAXLEN_IMPL_ID
			#endif
		#endif /* SOMGD_SOMD_MAXLEN_IMPL_ID */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ImplementationDef_SOMD_MAXLEN_IMPL_ID */
#ifndef ImplementationDef_SOMD_MAXLEN_IMPL_ALIAS
	#define ImplementationDef_SOMD_MAXLEN_IMPL_ALIAS   255L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SOMD_MAXLEN_IMPL_ALIAS
			#if defined(SOMD_MAXLEN_IMPL_ALIAS)
				#undef SOMD_MAXLEN_IMPL_ALIAS
				#define SOMGD_SOMD_MAXLEN_IMPL_ALIAS
			#else
				#define SOMD_MAXLEN_IMPL_ALIAS ImplementationDef_SOMD_MAXLEN_IMPL_ALIAS
			#endif
		#endif /* SOMGD_SOMD_MAXLEN_IMPL_ALIAS */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ImplementationDef_SOMD_MAXLEN_IMPL_ALIAS */
#ifndef ImplementationDef_SOMD_MAXLEN_IMPL_PROGRAM
	#define ImplementationDef_SOMD_MAXLEN_IMPL_PROGRAM   255L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SOMD_MAXLEN_IMPL_PROGRAM
			#if defined(SOMD_MAXLEN_IMPL_PROGRAM)
				#undef SOMD_MAXLEN_IMPL_PROGRAM
				#define SOMGD_SOMD_MAXLEN_IMPL_PROGRAM
			#else
				#define SOMD_MAXLEN_IMPL_PROGRAM ImplementationDef_SOMD_MAXLEN_IMPL_PROGRAM
			#endif
		#endif /* SOMGD_SOMD_MAXLEN_IMPL_PROGRAM */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ImplementationDef_SOMD_MAXLEN_IMPL_PROGRAM */
#ifndef ImplementationDef_SOMD_MAXLEN_FILENAME
	#define ImplementationDef_SOMD_MAXLEN_FILENAME   255L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SOMD_MAXLEN_FILENAME
			#if defined(SOMD_MAXLEN_FILENAME)
				#undef SOMD_MAXLEN_FILENAME
				#define SOMGD_SOMD_MAXLEN_FILENAME
			#else
				#define SOMD_MAXLEN_FILENAME ImplementationDef_SOMD_MAXLEN_FILENAME
			#endif
		#endif /* SOMGD_SOMD_MAXLEN_FILENAME */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ImplementationDef_SOMD_MAXLEN_FILENAME */
#ifndef ImplementationDef_SOMD_MAXLEN_HOSTNAME
	#define ImplementationDef_SOMD_MAXLEN_HOSTNAME   32L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SOMD_MAXLEN_HOSTNAME
			#if defined(SOMD_MAXLEN_HOSTNAME)
				#undef SOMD_MAXLEN_HOSTNAME
				#define SOMGD_SOMD_MAXLEN_HOSTNAME
			#else
				#define SOMD_MAXLEN_HOSTNAME ImplementationDef_SOMD_MAXLEN_HOSTNAME
			#endif
		#endif /* SOMGD_SOMD_MAXLEN_HOSTNAME */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ImplementationDef_SOMD_MAXLEN_HOSTNAME */
#ifndef ImplementationDef_SOMD_MAXLEN_CLASSNAME
	#define ImplementationDef_SOMD_MAXLEN_CLASSNAME   255L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SOMD_MAXLEN_CLASSNAME
			#if defined(SOMD_MAXLEN_CLASSNAME)
				#undef SOMD_MAXLEN_CLASSNAME
				#define SOMGD_SOMD_MAXLEN_CLASSNAME
			#else
				#define SOMD_MAXLEN_CLASSNAME ImplementationDef_SOMD_MAXLEN_CLASSNAME
			#endif
		#endif /* SOMGD_SOMD_MAXLEN_CLASSNAME */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ImplementationDef_SOMD_MAXLEN_CLASSNAME */
	typedef _IDL_SEQUENCE_octet *ImplementationDef_octet_seqP;
	#ifndef _IDL_SEQUENCE_ImplementationDef_octet_seqP_defined
		#define _IDL_SEQUENCE_ImplementationDef_octet_seqP_defined
		SOM_SEQUENCE_TYPEDEF(ImplementationDef_octet_seqP);
	#endif /* _IDL_SEQUENCE_ImplementationDef_octet_seqP_defined */
	typedef struct ImplementationDef_prot_response
	{
		string protocol;
		_IDL_SEQUENCE_string *response;
	} ImplementationDef_prot_response;
	#ifndef _IDL_SEQUENCE_ImplementationDef_prot_response_defined
		#define _IDL_SEQUENCE_ImplementationDef_prot_response_defined
		SOM_SEQUENCE_TYPEDEF(ImplementationDef_prot_response);
	#endif /* _IDL_SEQUENCE_ImplementationDef_prot_response_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_prot_response
			#if defined(prot_response)
				#undef prot_response
				#define SOMGD_prot_response
			#else
				#define prot_response ImplementationDef_prot_response
			#endif
		#endif /* SOMGD_prot_response */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_prot_response_defined
			#define _IDL_SEQUENCE_prot_response_defined
			#define _IDL_SEQUENCE_prot_response _IDL_SEQUENCE_ImplementationDef_prot_response
		#endif /* _IDL_SEQUENCE_prot_response_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	typedef _IDL_SEQUENCE_ImplementationDef_prot_response ImplementationDef_seq_prot_responses;
	#ifndef _IDL_SEQUENCE_ImplementationDef_seq_prot_responses_defined
		#define _IDL_SEQUENCE_ImplementationDef_seq_prot_responses_defined
		SOM_SEQUENCE_TYPEDEF(ImplementationDef_seq_prot_responses);
	#endif /* _IDL_SEQUENCE_ImplementationDef_seq_prot_responses_defined */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_id)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_id);
		#pragma linkage(somTP_ImplementationDef__set_impl_id,system)
		typedef somTP_ImplementationDef__set_impl_id *somTD_ImplementationDef__set_impl_id;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_id)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_id);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impl_id)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_id,system)
		typedef somTP_ImplementationDef__get_impl_id *somTD_ImplementationDef__get_impl_id;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impl_id)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_alias)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_alias);
		#pragma linkage(somTP_ImplementationDef__set_impl_alias,system)
		typedef somTP_ImplementationDef__set_impl_alias *somTD_ImplementationDef__set_impl_alias;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_alias)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_alias);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impl_alias)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_alias,system)
		typedef somTP_ImplementationDef__get_impl_alias *somTD_ImplementationDef__get_impl_alias;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impl_alias)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_program)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_program);
		#pragma linkage(somTP_ImplementationDef__set_impl_program,system)
		typedef somTP_ImplementationDef__set_impl_program *somTD_ImplementationDef__set_impl_program;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_program)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_program);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impl_program)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_program,system)
		typedef somTP_ImplementationDef__get_impl_program *somTD_ImplementationDef__get_impl_program;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impl_program)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_flags)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags impl_flags);
		#pragma linkage(somTP_ImplementationDef__set_impl_flags,system)
		typedef somTP_ImplementationDef__set_impl_flags *somTD_ImplementationDef__set_impl_flags;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_flags)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags impl_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Flags (somTP_ImplementationDef__get_impl_flags)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_flags,system)
		typedef somTP_ImplementationDef__get_impl_flags *somTD_ImplementationDef__get_impl_flags;
	#else /* __IBMC__ */
		typedef Flags (SOMLINK * somTD_ImplementationDef__get_impl_flags)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_server_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_server_class);
		#pragma linkage(somTP_ImplementationDef__set_impl_server_class,system)
		typedef somTP_ImplementationDef__set_impl_server_class *somTD_ImplementationDef__set_impl_server_class;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_server_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_server_class);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impl_server_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_server_class,system)
		typedef somTP_ImplementationDef__get_impl_server_class *somTD_ImplementationDef__get_impl_server_class;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impl_server_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_refdata_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_refdata_file);
		#pragma linkage(somTP_ImplementationDef__set_impl_refdata_file,system)
		typedef somTP_ImplementationDef__set_impl_refdata_file *somTD_ImplementationDef__set_impl_refdata_file;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_refdata_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_refdata_file);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impl_refdata_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_refdata_file,system)
		typedef somTP_ImplementationDef__get_impl_refdata_file *somTD_ImplementationDef__get_impl_refdata_file;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impl_refdata_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_refdata_bkup)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_refdata_bkup);
		#pragma linkage(somTP_ImplementationDef__set_impl_refdata_bkup,system)
		typedef somTP_ImplementationDef__set_impl_refdata_bkup *somTD_ImplementationDef__set_impl_refdata_bkup;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_refdata_bkup)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_refdata_bkup);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impl_refdata_bkup)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_refdata_bkup,system)
		typedef somTP_ImplementationDef__get_impl_refdata_bkup *somTD_ImplementationDef__get_impl_refdata_bkup;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impl_refdata_bkup)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_hostname)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_hostname);
		#pragma linkage(somTP_ImplementationDef__set_impl_hostname,system)
		typedef somTP_ImplementationDef__set_impl_hostname *somTD_ImplementationDef__set_impl_hostname;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_hostname)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impl_hostname);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impl_hostname)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_hostname,system)
		typedef somTP_ImplementationDef__get_impl_hostname *somTD_ImplementationDef__get_impl_hostname;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impl_hostname)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_config_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string config_file);
		#pragma linkage(somTP_ImplementationDef__set_config_file,system)
		typedef somTP_ImplementationDef__set_config_file *somTD_ImplementationDef__set_config_file;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_config_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string config_file);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_config_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_config_file,system)
		typedef somTP_ImplementationDef__get_config_file *somTD_ImplementationDef__get_config_file;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_config_file)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impldef_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impldef_class);
		#pragma linkage(somTP_ImplementationDef__set_impldef_class,system)
		typedef somTP_ImplementationDef__set_impldef_class *somTD_ImplementationDef__set_impldef_class;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impldef_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string impldef_class);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_impldef_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impldef_class,system)
		typedef somTP_ImplementationDef__get_impldef_class *somTD_ImplementationDef__get_impldef_class;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_impldef_class)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_svr_objref)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef_octet_seqP svr_objref);
		#pragma linkage(somTP_ImplementationDef__set_svr_objref,system)
		typedef somTP_ImplementationDef__set_svr_objref *somTD_ImplementationDef__set_svr_objref;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_svr_objref)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef_octet_seqP svr_objref);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ImplementationDef_octet_seqP (somTP_ImplementationDef__get_svr_objref)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_svr_objref,system)
		typedef somTP_ImplementationDef__get_svr_objref *somTD_ImplementationDef__get_svr_objref;
	#else /* __IBMC__ */
		typedef ImplementationDef_octet_seqP (SOMLINK * somTD_ImplementationDef__get_svr_objref)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_protocols)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string protocols);
		#pragma linkage(somTP_ImplementationDef__set_protocols,system)
		typedef somTP_ImplementationDef__set_protocols *somTD_ImplementationDef__set_protocols;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_protocols)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ string protocols);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ImplementationDef__get_protocols)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_protocols,system)
		typedef somTP_ImplementationDef__get_protocols *somTD_ImplementationDef__get_protocols;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ImplementationDef__get_protocols)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_registrar_resp)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef_seq_prot_responses *registrar_resp);
		#pragma linkage(somTP_ImplementationDef__set_registrar_resp,system)
		typedef somTP_ImplementationDef__set_registrar_resp *somTD_ImplementationDef__set_registrar_resp;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_registrar_resp)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef_seq_prot_responses *registrar_resp);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ImplementationDef_seq_prot_responses *(somTP_ImplementationDef__get_registrar_resp)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_registrar_resp,system)
		typedef somTP_ImplementationDef__get_registrar_resp *somTD_ImplementationDef__get_registrar_resp;
	#else /* __IBMC__ */
		typedef ImplementationDef_seq_prot_responses *(SOMLINK * somTD_ImplementationDef__get_registrar_resp)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplementationDef__set_impl_version)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned short impl_version);
		#pragma linkage(somTP_ImplementationDef__set_impl_version,system)
		typedef somTP_ImplementationDef__set_impl_version *somTD_ImplementationDef__set_impl_version;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplementationDef__set_impl_version)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned short impl_version);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned short (somTP_ImplementationDef__get_impl_version)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ImplementationDef__get_impl_version,system)
		typedef somTP_ImplementationDef__get_impl_version *somTD_ImplementationDef__get_impl_version;
	#else /* __IBMC__ */
		typedef unsigned short (SOMLINK * somTD_ImplementationDef__get_impl_version)(
			ImplementationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef ImplementationDef_MajorVersion
		#define ImplementationDef_MajorVersion   2
	#endif /* ImplementationDef_MajorVersion */
	#ifndef ImplementationDef_MinorVersion
		#define ImplementationDef_MinorVersion   2
	#endif /* ImplementationDef_MinorVersion */
	typedef struct ImplementationDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _set_impl_id;
		somMToken _get_impl_id;
		somMToken _set_impl_alias;
		somMToken _get_impl_alias;
		somMToken _set_impl_program;
		somMToken _get_impl_program;
		somMToken _set_impl_flags;
		somMToken _get_impl_flags;
		somMToken _set_impl_server_class;
		somMToken _get_impl_server_class;
		somMToken _set_impl_refdata_file;
		somMToken _get_impl_refdata_file;
		somMToken _set_impl_refdata_bkup;
		somMToken _get_impl_refdata_bkup;
		somMToken _set_impl_hostname;
		somMToken _get_impl_hostname;
		somMToken private16;
		somMToken private17;
		somMToken private18;
		somMToken _set_config_file;
		somMToken _get_config_file;
		somMToken _set_impldef_class;
		somMToken _get_impldef_class;
		somMToken _set_svr_objref;
		somMToken _get_svr_objref;
		somMToken _set_protocols;
		somMToken _get_protocols;
		somMToken _set_registrar_resp;
		somMToken _get_registrar_resp;
		somMToken _set_impl_version;
		somMToken _get_impl_version;
		somMToken private31;
		somMToken private32;
	} ImplementationDefClassDataStructure;
	typedef struct ImplementationDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ImplementationDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_impldef_Source) || defined(ImplementationDef_Class_Source)
			SOMEXTERN struct ImplementationDefClassDataStructure _ImplementationDefClassData;
			#ifndef ImplementationDefClassData
				#define ImplementationDefClassData    _ImplementationDefClassData
			#endif /* ImplementationDefClassData */
		#else
			SOMEXTERN struct ImplementationDefClassDataStructure * SOMLINK resolve_ImplementationDefClassData(void);
			#ifndef ImplementationDefClassData
				#define ImplementationDefClassData    (*(resolve_ImplementationDefClassData()))
			#endif /* ImplementationDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_impldef_Source) || defined(ImplementationDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_impldef_Source || ImplementationDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_impldef_Source || ImplementationDef_Class_Source */
		struct ImplementationDefClassDataStructure SOMDLINK ImplementationDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_impldef_Source) || defined(ImplementationDef_Class_Source)
			SOMEXTERN struct ImplementationDefCClassDataStructure _ImplementationDefCClassData;
			#ifndef ImplementationDefCClassData
				#define ImplementationDefCClassData    _ImplementationDefCClassData
			#endif /* ImplementationDefCClassData */
		#else
			SOMEXTERN struct ImplementationDefCClassDataStructure * SOMLINK resolve_ImplementationDefCClassData(void);
			#ifndef ImplementationDefCClassData
				#define ImplementationDefCClassData    (*(resolve_ImplementationDefCClassData()))
			#endif /* ImplementationDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_impldef_Source) || defined(ImplementationDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_impldef_Source || ImplementationDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_impldef_Source || ImplementationDef_Class_Source */
		struct ImplementationDefCClassDataStructure SOMDLINK ImplementationDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_impldef_Source) || defined(ImplementationDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_impldef_Source || ImplementationDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_impldef_Source || ImplementationDef_Class_Source */
	SOMClass SOMSTAR SOMLINK ImplementationDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ImplementationDef (ImplementationDefClassData.classObject)
	#ifndef SOMGD_ImplementationDef
		#if (defined(_ImplementationDef) || defined(__ImplementationDef))
			#undef _ImplementationDef
			#undef __ImplementationDef
			#define SOMGD_ImplementationDef 1
		#else
			#define _ImplementationDef _SOMCLASS_ImplementationDef
		#endif /* _ImplementationDef */
	#endif /* SOMGD_ImplementationDef */
	#define ImplementationDef_classObj _SOMCLASS_ImplementationDef
	#define _SOMMTOKEN_ImplementationDef(method) ((somMToken)(ImplementationDefClassData.method))
	#ifndef ImplementationDefNew
		#define ImplementationDefNew() ( _ImplementationDef ? \
			(SOMClass_somNew(_ImplementationDef)) : \
			( ImplementationDefNewClass( \
				ImplementationDef_MajorVersion, \
				ImplementationDef_MinorVersion), \
			SOMClass_somNew(_ImplementationDef))) 
	#endif /* NewImplementationDef */
	#ifndef ImplementationDef__set_impl_id
		#define ImplementationDef__set_impl_id(somSelf,ev,impl_id) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_id)  \
				(somSelf,ev,impl_id)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_id
				#if defined(__set_impl_id)
					#undef __set_impl_id
					#define SOMGD___set_impl_id
				#else
					#define __set_impl_id ImplementationDef__set_impl_id
				#endif
			#endif /* SOMGD___set_impl_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_id */
	#ifndef ImplementationDef__get_impl_id
		#define ImplementationDef__get_impl_id(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_id)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_id
				#if defined(__get_impl_id)
					#undef __get_impl_id
					#define SOMGD___get_impl_id
				#else
					#define __get_impl_id ImplementationDef__get_impl_id
				#endif
			#endif /* SOMGD___get_impl_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_id */
	#ifndef ImplementationDef__set_impl_alias
		#define ImplementationDef__set_impl_alias(somSelf,ev,impl_alias) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_alias)  \
				(somSelf,ev,impl_alias)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_alias
				#if defined(__set_impl_alias)
					#undef __set_impl_alias
					#define SOMGD___set_impl_alias
				#else
					#define __set_impl_alias ImplementationDef__set_impl_alias
				#endif
			#endif /* SOMGD___set_impl_alias */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_alias */
	#ifndef ImplementationDef__get_impl_alias
		#define ImplementationDef__get_impl_alias(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_alias)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_alias
				#if defined(__get_impl_alias)
					#undef __get_impl_alias
					#define SOMGD___get_impl_alias
				#else
					#define __get_impl_alias ImplementationDef__get_impl_alias
				#endif
			#endif /* SOMGD___get_impl_alias */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_alias */
	#ifndef ImplementationDef__set_impl_program
		#define ImplementationDef__set_impl_program(somSelf,ev,impl_program) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_program)  \
				(somSelf,ev,impl_program)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_program
				#if defined(__set_impl_program)
					#undef __set_impl_program
					#define SOMGD___set_impl_program
				#else
					#define __set_impl_program ImplementationDef__set_impl_program
				#endif
			#endif /* SOMGD___set_impl_program */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_program */
	#ifndef ImplementationDef__get_impl_program
		#define ImplementationDef__get_impl_program(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_program)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_program
				#if defined(__get_impl_program)
					#undef __get_impl_program
					#define SOMGD___get_impl_program
				#else
					#define __get_impl_program ImplementationDef__get_impl_program
				#endif
			#endif /* SOMGD___get_impl_program */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_program */
	#ifndef ImplementationDef__set_impl_flags
		#define ImplementationDef__set_impl_flags(somSelf,ev,impl_flags) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_flags)  \
				(somSelf,ev,impl_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_flags
				#if defined(__set_impl_flags)
					#undef __set_impl_flags
					#define SOMGD___set_impl_flags
				#else
					#define __set_impl_flags ImplementationDef__set_impl_flags
				#endif
			#endif /* SOMGD___set_impl_flags */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_flags */
	#ifndef ImplementationDef__get_impl_flags
		#define ImplementationDef__get_impl_flags(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_flags)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_flags
				#if defined(__get_impl_flags)
					#undef __get_impl_flags
					#define SOMGD___get_impl_flags
				#else
					#define __get_impl_flags ImplementationDef__get_impl_flags
				#endif
			#endif /* SOMGD___get_impl_flags */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_flags */
	#ifndef ImplementationDef__set_impl_server_class
		#define ImplementationDef__set_impl_server_class(somSelf,ev,impl_server_class) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_server_class)  \
				(somSelf,ev,impl_server_class)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_server_class
				#if defined(__set_impl_server_class)
					#undef __set_impl_server_class
					#define SOMGD___set_impl_server_class
				#else
					#define __set_impl_server_class ImplementationDef__set_impl_server_class
				#endif
			#endif /* SOMGD___set_impl_server_class */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_server_class */
	#ifndef ImplementationDef__get_impl_server_class
		#define ImplementationDef__get_impl_server_class(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_server_class)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_server_class
				#if defined(__get_impl_server_class)
					#undef __get_impl_server_class
					#define SOMGD___get_impl_server_class
				#else
					#define __get_impl_server_class ImplementationDef__get_impl_server_class
				#endif
			#endif /* SOMGD___get_impl_server_class */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_server_class */
	#ifndef ImplementationDef__set_impl_refdata_file
		#define ImplementationDef__set_impl_refdata_file(somSelf,ev,impl_refdata_file) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_refdata_file)  \
				(somSelf,ev,impl_refdata_file)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_refdata_file
				#if defined(__set_impl_refdata_file)
					#undef __set_impl_refdata_file
					#define SOMGD___set_impl_refdata_file
				#else
					#define __set_impl_refdata_file ImplementationDef__set_impl_refdata_file
				#endif
			#endif /* SOMGD___set_impl_refdata_file */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_refdata_file */
	#ifndef ImplementationDef__get_impl_refdata_file
		#define ImplementationDef__get_impl_refdata_file(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_refdata_file)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_refdata_file
				#if defined(__get_impl_refdata_file)
					#undef __get_impl_refdata_file
					#define SOMGD___get_impl_refdata_file
				#else
					#define __get_impl_refdata_file ImplementationDef__get_impl_refdata_file
				#endif
			#endif /* SOMGD___get_impl_refdata_file */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_refdata_file */
	#ifndef ImplementationDef__set_impl_refdata_bkup
		#define ImplementationDef__set_impl_refdata_bkup(somSelf,ev,impl_refdata_bkup) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_refdata_bkup)  \
				(somSelf,ev,impl_refdata_bkup)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_refdata_bkup
				#if defined(__set_impl_refdata_bkup)
					#undef __set_impl_refdata_bkup
					#define SOMGD___set_impl_refdata_bkup
				#else
					#define __set_impl_refdata_bkup ImplementationDef__set_impl_refdata_bkup
				#endif
			#endif /* SOMGD___set_impl_refdata_bkup */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_refdata_bkup */
	#ifndef ImplementationDef__get_impl_refdata_bkup
		#define ImplementationDef__get_impl_refdata_bkup(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_refdata_bkup)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_refdata_bkup
				#if defined(__get_impl_refdata_bkup)
					#undef __get_impl_refdata_bkup
					#define SOMGD___get_impl_refdata_bkup
				#else
					#define __get_impl_refdata_bkup ImplementationDef__get_impl_refdata_bkup
				#endif
			#endif /* SOMGD___get_impl_refdata_bkup */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_refdata_bkup */
	#ifndef ImplementationDef__set_impl_hostname
		#define ImplementationDef__set_impl_hostname(somSelf,ev,impl_hostname) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_hostname)  \
				(somSelf,ev,impl_hostname)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_hostname
				#if defined(__set_impl_hostname)
					#undef __set_impl_hostname
					#define SOMGD___set_impl_hostname
				#else
					#define __set_impl_hostname ImplementationDef__set_impl_hostname
				#endif
			#endif /* SOMGD___set_impl_hostname */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_hostname */
	#ifndef ImplementationDef__get_impl_hostname
		#define ImplementationDef__get_impl_hostname(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_hostname)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_hostname
				#if defined(__get_impl_hostname)
					#undef __get_impl_hostname
					#define SOMGD___get_impl_hostname
				#else
					#define __get_impl_hostname ImplementationDef__get_impl_hostname
				#endif
			#endif /* SOMGD___get_impl_hostname */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_hostname */
	#ifndef ImplementationDef__set_config_file
		#define ImplementationDef__set_config_file(somSelf,ev,config_file) \
			SOM_Resolve(somSelf,ImplementationDef,_set_config_file)  \
				(somSelf,ev,config_file)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_config_file
				#if defined(__set_config_file)
					#undef __set_config_file
					#define SOMGD___set_config_file
				#else
					#define __set_config_file ImplementationDef__set_config_file
				#endif
			#endif /* SOMGD___set_config_file */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_config_file */
	#ifndef ImplementationDef__get_config_file
		#define ImplementationDef__get_config_file(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_config_file)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_config_file
				#if defined(__get_config_file)
					#undef __get_config_file
					#define SOMGD___get_config_file
				#else
					#define __get_config_file ImplementationDef__get_config_file
				#endif
			#endif /* SOMGD___get_config_file */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_config_file */
	#ifndef ImplementationDef__set_impldef_class
		#define ImplementationDef__set_impldef_class(somSelf,ev,impldef_class) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impldef_class)  \
				(somSelf,ev,impldef_class)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impldef_class
				#if defined(__set_impldef_class)
					#undef __set_impldef_class
					#define SOMGD___set_impldef_class
				#else
					#define __set_impldef_class ImplementationDef__set_impldef_class
				#endif
			#endif /* SOMGD___set_impldef_class */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impldef_class */
	#ifndef ImplementationDef__get_impldef_class
		#define ImplementationDef__get_impldef_class(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impldef_class)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impldef_class
				#if defined(__get_impldef_class)
					#undef __get_impldef_class
					#define SOMGD___get_impldef_class
				#else
					#define __get_impldef_class ImplementationDef__get_impldef_class
				#endif
			#endif /* SOMGD___get_impldef_class */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impldef_class */
	#ifndef ImplementationDef__set_svr_objref
		#define ImplementationDef__set_svr_objref(somSelf,ev,svr_objref) \
			SOM_Resolve(somSelf,ImplementationDef,_set_svr_objref)  \
				(somSelf,ev,svr_objref)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_svr_objref
				#if defined(__set_svr_objref)
					#undef __set_svr_objref
					#define SOMGD___set_svr_objref
				#else
					#define __set_svr_objref ImplementationDef__set_svr_objref
				#endif
			#endif /* SOMGD___set_svr_objref */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_svr_objref */
	#ifndef ImplementationDef__get_svr_objref
		#define ImplementationDef__get_svr_objref(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_svr_objref)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_svr_objref
				#if defined(__get_svr_objref)
					#undef __get_svr_objref
					#define SOMGD___get_svr_objref
				#else
					#define __get_svr_objref ImplementationDef__get_svr_objref
				#endif
			#endif /* SOMGD___get_svr_objref */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_svr_objref */
	#ifndef ImplementationDef__set_protocols
		#define ImplementationDef__set_protocols(somSelf,ev,protocols) \
			SOM_Resolve(somSelf,ImplementationDef,_set_protocols)  \
				(somSelf,ev,protocols)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_protocols
				#if defined(__set_protocols)
					#undef __set_protocols
					#define SOMGD___set_protocols
				#else
					#define __set_protocols ImplementationDef__set_protocols
				#endif
			#endif /* SOMGD___set_protocols */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_protocols */
	#ifndef ImplementationDef__get_protocols
		#define ImplementationDef__get_protocols(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_protocols)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_protocols
				#if defined(__get_protocols)
					#undef __get_protocols
					#define SOMGD___get_protocols
				#else
					#define __get_protocols ImplementationDef__get_protocols
				#endif
			#endif /* SOMGD___get_protocols */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_protocols */
	#ifndef ImplementationDef__set_registrar_resp
		#define ImplementationDef__set_registrar_resp(somSelf,ev,registrar_resp) \
			SOM_Resolve(somSelf,ImplementationDef,_set_registrar_resp)  \
				(somSelf,ev,registrar_resp)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_registrar_resp
				#if defined(__set_registrar_resp)
					#undef __set_registrar_resp
					#define SOMGD___set_registrar_resp
				#else
					#define __set_registrar_resp ImplementationDef__set_registrar_resp
				#endif
			#endif /* SOMGD___set_registrar_resp */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_registrar_resp */
	#ifndef ImplementationDef__get_registrar_resp
		#define ImplementationDef__get_registrar_resp(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_registrar_resp)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_registrar_resp
				#if defined(__get_registrar_resp)
					#undef __get_registrar_resp
					#define SOMGD___get_registrar_resp
				#else
					#define __get_registrar_resp ImplementationDef__get_registrar_resp
				#endif
			#endif /* SOMGD___get_registrar_resp */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_registrar_resp */
	#ifndef ImplementationDef__set_impl_version
		#define ImplementationDef__set_impl_version(somSelf,ev,impl_version) \
			SOM_Resolve(somSelf,ImplementationDef,_set_impl_version)  \
				(somSelf,ev,impl_version)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_impl_version
				#if defined(__set_impl_version)
					#undef __set_impl_version
					#define SOMGD___set_impl_version
				#else
					#define __set_impl_version ImplementationDef__set_impl_version
				#endif
			#endif /* SOMGD___set_impl_version */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__set_impl_version */
	#ifndef ImplementationDef__get_impl_version
		#define ImplementationDef__get_impl_version(somSelf,ev) \
			SOM_Resolve(somSelf,ImplementationDef,_get_impl_version)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_impl_version
				#if defined(__get_impl_version)
					#undef __get_impl_version
					#define SOMGD___get_impl_version
				#else
					#define __get_impl_version ImplementationDef__get_impl_version
				#endif
			#endif /* SOMGD___get_impl_version */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplementationDef__get_impl_version */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ImplementationDef_somInit SOMObject_somInit
#define ImplementationDef_somUninit SOMObject_somUninit
#define ImplementationDef_somFree SOMObject_somFree
#define ImplementationDef_somGetClass SOMObject_somGetClass
#define ImplementationDef_somGetClassName SOMObject_somGetClassName
#define ImplementationDef_somGetSize SOMObject_somGetSize
#define ImplementationDef_somIsA SOMObject_somIsA
#define ImplementationDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define ImplementationDef_somRespondsTo SOMObject_somRespondsTo
#define ImplementationDef_somDispatch SOMObject_somDispatch
#define ImplementationDef_somClassDispatch SOMObject_somClassDispatch
#define ImplementationDef_somCastObj SOMObject_somCastObj
#define ImplementationDef_somResetObj SOMObject_somResetObj
#define ImplementationDef_somPrintSelf SOMObject_somPrintSelf
#define ImplementationDef_somDumpSelf SOMObject_somDumpSelf
#define ImplementationDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define ImplementationDef_somDefaultInit SOMObject_somDefaultInit
#define ImplementationDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ImplementationDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ImplementationDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ImplementationDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ImplementationDef_somDefaultAssign SOMObject_somDefaultAssign
#define ImplementationDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ImplementationDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define ImplementationDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ImplementationDef_somDestruct SOMObject_somDestruct
#define ImplementationDef__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define ImplementationDef_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#define ImplementationDef_externalize_to_stream CosStream_Streamable_externalize_to_stream
#define ImplementationDef_internalize_from_stream CosStream_Streamable_internalize_from_stream
#define ImplementationDef__get_external_form_id CosStream_Streamable__get_external_form_id
#endif /* SOM_DONT_USE_INHERITED_MACROS */


#define IMPLDEF_MULTI_THREAD  0x00000001
#define IMPLDEF_DISABLE_SVR   0x00000004
#define IMPLDEF_IMPLID_SET    0x00000008
#define IMPLDEF_SECUREMODE    0x00000020
#define IMPLDEF_NONSTOPPABLE  0x00000010
#define IMPLDEF_MANAGED       0x00000040


#endif /* SOM_Module_impldef_Header_h */
