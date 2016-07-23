/* generated from xmscssae.idl */
/* internal conditional is SOM_Module_xmscssae_Source */
#ifndef SOM_Module_xmscssae_Header_h
	#define SOM_Module_xmscssae_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_SysAdminException_defined
		#define _IDL_SysAdminException_defined
		typedef SOMObject SysAdminException;
	#endif /* _IDL_SysAdminException_defined */
	#ifndef _IDL_SEQUENCE_SysAdminException_defined
		#define _IDL_SEQUENCE_SysAdminException_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SysAdminException SOMSTAR ,sequence(SysAdminException));
	#endif /* _IDL_SEQUENCE_SysAdminException_defined */
#define ex_SysAdminException_Exception   "::SysAdminException::Exception"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_Exception
#ifndef ex_Exception
#define ex_Exception  ex_SysAdminException_Exception
#else
#define SOMTGD_ex_Exception
#undef ex_Exception
#endif /* ex_Exception */
#endif /* SOMTGD_ex_Exception */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_Exception
{
	somToken dummy;
} SysAdminException_Exception;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_Exception
		#if defined(Exception)
			#undef Exception
			#define SOMGD_Exception
		#else
			#define Exception SysAdminException_Exception
		#endif
	#endif /* SOMGD_Exception */
#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef _IDL_SEQUENCE_octet_defined
		#define _IDL_SEQUENCE_octet_defined
		SOM_SEQUENCE_TYPEDEF(octet);
	#endif /* _IDL_SEQUENCE_octet_defined */
	typedef _IDL_SEQUENCE_octet SysAdminException_MsgContext;
	#ifndef _IDL_SEQUENCE_SysAdminException_MsgContext_defined
		#define _IDL_SEQUENCE_SysAdminException_MsgContext_defined
		SOM_SEQUENCE_TYPEDEF(SysAdminException_MsgContext);
	#endif /* _IDL_SEQUENCE_SysAdminException_MsgContext_defined */
#define ex_SysAdminException_ExException   "::SysAdminException::ExException"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExException
#ifndef ex_ExException
#define ex_ExException  ex_SysAdminException_ExException
#else
#define SOMTGD_ex_ExException
#undef ex_ExException
#endif /* ex_ExException */
#endif /* SOMTGD_ex_ExException */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExException
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
} SysAdminException_ExException;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExException
		#if defined(ExException)
			#undef ExException
			#define SOMGD_ExException
		#else
			#define ExException SysAdminException_ExException
		#endif
	#endif /* SOMGD_ExException */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_SysAdminException_ExFailed   "::SysAdminException::ExFailed"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExFailed
#ifndef ex_ExFailed
#define ex_ExFailed  ex_SysAdminException_ExFailed
#else
#define SOMTGD_ex_ExFailed
#undef ex_ExFailed
#endif /* ex_ExFailed */
#endif /* SOMTGD_ex_ExFailed */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExFailed
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
	string operation_name;
} SysAdminException_ExFailed;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExFailed
		#if defined(ExFailed)
			#undef ExFailed
			#define SOMGD_ExFailed
		#else
			#define ExFailed SysAdminException_ExFailed
		#endif
	#endif /* SOMGD_ExFailed */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_SysAdminException_ExInvalid   "::SysAdminException::ExInvalid"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExInvalid
#ifndef ex_ExInvalid
#define ex_ExInvalid  ex_SysAdminException_ExInvalid
#else
#define SOMTGD_ex_ExInvalid
#undef ex_ExInvalid
#endif /* ex_ExInvalid */
#endif /* SOMTGD_ex_ExInvalid */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExInvalid
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
	string resource_name;
} SysAdminException_ExInvalid;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExInvalid
		#if defined(ExInvalid)
			#undef ExInvalid
			#define SOMGD_ExInvalid
		#else
			#define ExInvalid SysAdminException_ExInvalid
		#endif
	#endif /* SOMGD_ExInvalid */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_SysAdminException_ExNotFound   "::SysAdminException::ExNotFound"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExNotFound
#ifndef ex_ExNotFound
#define ex_ExNotFound  ex_SysAdminException_ExNotFound
#else
#define SOMTGD_ex_ExNotFound
#undef ex_ExNotFound
#endif /* ex_ExNotFound */
#endif /* SOMTGD_ex_ExNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExNotFound
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
	string resource_name;
} SysAdminException_ExNotFound;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExNotFound
		#if defined(ExNotFound)
			#undef ExNotFound
			#define SOMGD_ExNotFound
		#else
			#define ExNotFound SysAdminException_ExNotFound
		#endif
	#endif /* SOMGD_ExNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_SysAdminException_ExExists   "::SysAdminException::ExExists"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExExists
#ifndef ex_ExExists
#define ex_ExExists  ex_SysAdminException_ExExists
#else
#define SOMTGD_ex_ExExists
#undef ex_ExExists
#endif /* ex_ExExists */
#endif /* SOMTGD_ex_ExExists */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExExists
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
	string resource_name;
} SysAdminException_ExExists;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExExists
		#if defined(ExExists)
			#undef ExExists
			#define SOMGD_ExExists
		#else
			#define ExExists SysAdminException_ExExists
		#endif
	#endif /* SOMGD_ExExists */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_SysAdminException_ExFileIO   "::SysAdminException::ExFileIO"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExFileIO
#ifndef ex_ExFileIO
#define ex_ExFileIO  ex_SysAdminException_ExFileIO
#else
#define SOMTGD_ex_ExFileIO
#undef ex_ExFileIO
#endif /* ex_ExFileIO */
#endif /* SOMTGD_ex_ExFileIO */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExFileIO
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
	string resource_name;
} SysAdminException_ExFileIO;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExFileIO
		#if defined(ExFileIO)
			#undef ExFileIO
			#define SOMGD_ExFileIO
		#else
			#define ExFileIO SysAdminException_ExFileIO
		#endif
	#endif /* SOMGD_ExFileIO */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_SysAdminException_ExObjNotFound   "::SysAdminException::ExObjNotFound"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExObjNotFound
#ifndef ex_ExObjNotFound
#define ex_ExObjNotFound  ex_SysAdminException_ExObjNotFound
#else
#define SOMTGD_ex_ExObjNotFound
#undef ex_ExObjNotFound
#endif /* ex_ExObjNotFound */
#endif /* SOMTGD_ex_ExObjNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExObjNotFound
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
	string resource_name;
} SysAdminException_ExObjNotFound;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExObjNotFound
		#if defined(ExObjNotFound)
			#undef ExObjNotFound
			#define SOMGD_ExObjNotFound
		#else
			#define ExObjNotFound SysAdminException_ExObjNotFound
		#endif
	#endif /* SOMGD_ExObjNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_SysAdminException_ExNoMoreMemory   "::SysAdminException::ExNoMoreMemory"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ExNoMoreMemory
#ifndef ex_ExNoMoreMemory
#define ex_ExNoMoreMemory  ex_SysAdminException_ExNoMoreMemory
#else
#define SOMTGD_ex_ExNoMoreMemory
#undef ex_ExNoMoreMemory
#endif /* ex_ExNoMoreMemory */
#endif /* SOMTGD_ex_ExNoMoreMemory */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct SysAdminException_ExNoMoreMemory
{
	string type_name;
	string catalog;
	long key;
	string default_msg;
	long time_stamp;
	SysAdminException_MsgContext msg_context;
	string message;
	long size;
} SysAdminException_ExNoMoreMemory;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ExNoMoreMemory
		#if defined(ExNoMoreMemory)
			#undef ExNoMoreMemory
			#define SOMGD_ExNoMoreMemory
		#else
			#define ExNoMoreMemory SysAdminException_ExNoMoreMemory
		#endif
	#endif /* SOMGD_ExNoMoreMemory */
#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SysAdminException_MajorVersion
		#define SysAdminException_MajorVersion   3
	#endif /* SysAdminException_MajorVersion */
	#ifndef SysAdminException_MinorVersion
		#define SysAdminException_MinorVersion   0
	#endif /* SysAdminException_MinorVersion */
	typedef struct SysAdminExceptionClassDataStructure
	{
		SOMClass SOMSTAR classObject;
	} SysAdminExceptionClassDataStructure;
	typedef struct SysAdminExceptionCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SysAdminExceptionCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_xmscssae_Source) || defined(SysAdminException_Class_Source)
			SOMEXTERN struct SysAdminExceptionClassDataStructure _SysAdminExceptionClassData;
			#ifndef SysAdminExceptionClassData
				#define SysAdminExceptionClassData    _SysAdminExceptionClassData
			#endif /* SysAdminExceptionClassData */
		#else
			SOMEXTERN struct SysAdminExceptionClassDataStructure * SOMLINK resolve_SysAdminExceptionClassData(void);
			#ifndef SysAdminExceptionClassData
				#define SysAdminExceptionClassData    (*(resolve_SysAdminExceptionClassData()))
			#endif /* SysAdminExceptionClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_xmscssae_Source) || defined(SysAdminException_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xmscssae_Source || SysAdminException_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xmscssae_Source || SysAdminException_Class_Source */
		struct SysAdminExceptionClassDataStructure SOMDLINK SysAdminExceptionClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_xmscssae_Source) || defined(SysAdminException_Class_Source)
			SOMEXTERN struct SysAdminExceptionCClassDataStructure _SysAdminExceptionCClassData;
			#ifndef SysAdminExceptionCClassData
				#define SysAdminExceptionCClassData    _SysAdminExceptionCClassData
			#endif /* SysAdminExceptionCClassData */
		#else
			SOMEXTERN struct SysAdminExceptionCClassDataStructure * SOMLINK resolve_SysAdminExceptionCClassData(void);
			#ifndef SysAdminExceptionCClassData
				#define SysAdminExceptionCClassData    (*(resolve_SysAdminExceptionCClassData()))
			#endif /* SysAdminExceptionCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_xmscssae_Source) || defined(SysAdminException_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xmscssae_Source || SysAdminException_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xmscssae_Source || SysAdminException_Class_Source */
		struct SysAdminExceptionCClassDataStructure SOMDLINK SysAdminExceptionCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_xmscssae_Source) || defined(SysAdminException_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_xmscssae_Source || SysAdminException_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_xmscssae_Source || SysAdminException_Class_Source */
	SOMClass SOMSTAR SOMLINK SysAdminExceptionNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SysAdminException (SysAdminExceptionClassData.classObject)
	#ifndef SOMGD_SysAdminException
		#if (defined(_SysAdminException) || defined(__SysAdminException))
			#undef _SysAdminException
			#undef __SysAdminException
			#define SOMGD_SysAdminException 1
		#else
			#define _SysAdminException _SOMCLASS_SysAdminException
		#endif /* _SysAdminException */
	#endif /* SOMGD_SysAdminException */
	#define SysAdminException_classObj _SOMCLASS_SysAdminException
	#define _SOMMTOKEN_SysAdminException(method) ((somMToken)(SysAdminExceptionClassData.method))
	#ifndef SysAdminExceptionNew
		#define SysAdminExceptionNew() ( _SysAdminException ? \
			(SOMClass_somNew(_SysAdminException)) : \
			( SysAdminExceptionNewClass( \
				SysAdminException_MajorVersion, \
				SysAdminException_MinorVersion), \
			SOMClass_somNew(_SysAdminException))) 
	#endif /* NewSysAdminException */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SysAdminException_somInit SOMObject_somInit
#define SysAdminException_somUninit SOMObject_somUninit
#define SysAdminException_somFree SOMObject_somFree
#define SysAdminException_somGetClass SOMObject_somGetClass
#define SysAdminException_somGetClassName SOMObject_somGetClassName
#define SysAdminException_somGetSize SOMObject_somGetSize
#define SysAdminException_somIsA SOMObject_somIsA
#define SysAdminException_somIsInstanceOf SOMObject_somIsInstanceOf
#define SysAdminException_somRespondsTo SOMObject_somRespondsTo
#define SysAdminException_somDispatch SOMObject_somDispatch
#define SysAdminException_somClassDispatch SOMObject_somClassDispatch
#define SysAdminException_somCastObj SOMObject_somCastObj
#define SysAdminException_somResetObj SOMObject_somResetObj
#define SysAdminException_somPrintSelf SOMObject_somPrintSelf
#define SysAdminException_somDumpSelf SOMObject_somDumpSelf
#define SysAdminException_somDumpSelfInt SOMObject_somDumpSelfInt
#define SysAdminException_somDefaultInit SOMObject_somDefaultInit
#define SysAdminException_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SysAdminException_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SysAdminException_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SysAdminException_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SysAdminException_somDefaultAssign SOMObject_somDefaultAssign
#define SysAdminException_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SysAdminException_somDefaultVAssign SOMObject_somDefaultVAssign
#define SysAdminException_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SysAdminException_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_xmscssae_Header_h */
