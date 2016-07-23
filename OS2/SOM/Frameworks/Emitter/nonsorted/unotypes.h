/* generated from unotypes.idl */
/* internal conditional is SOM_Module_unotypes_Source */
#ifndef SOM_Module_unotypes_Header_h
	#define SOM_Module_unotypes_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <containd.h>
	#include <somdtype.h>
	#ifndef _IDL_Principal_defined
		#define _IDL_Principal_defined
		typedef SOMObject Principal;
	#endif /* _IDL_Principal_defined */
	#ifndef _IDL_SEQUENCE_Principal_defined
		#define _IDL_SEQUENCE_Principal_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Principal SOMSTAR ,sequence(Principal));
	#endif /* _IDL_SEQUENCE_Principal_defined */
		#ifndef _IDL_IOP_Codec_defined
			#define _IDL_IOP_Codec_defined
			typedef SOMObject IOP_Codec;
		#endif /* _IDL_IOP_Codec_defined */
		#ifndef _IDL_SEQUENCE_IOP_Codec_defined
			#define _IDL_SEQUENCE_IOP_Codec_defined
			SOM_SEQUENCE_TYPEDEF_NAME(IOP_Codec SOMSTAR ,sequence(IOP_Codec));
		#endif /* _IDL_SEQUENCE_IOP_Codec_defined */
		#ifndef _IDL_IOP_CodecFactory_defined
			#define _IDL_IOP_CodecFactory_defined
			typedef SOMObject IOP_CodecFactory;
		#endif /* _IDL_IOP_CodecFactory_defined */
		#ifndef _IDL_SEQUENCE_IOP_CodecFactory_defined
			#define _IDL_SEQUENCE_IOP_CodecFactory_defined
			SOM_SEQUENCE_TYPEDEF_NAME(IOP_CodecFactory SOMSTAR ,sequence(IOP_CodecFactory));
		#endif /* _IDL_SEQUENCE_IOP_CodecFactory_defined */
		typedef unsigned long IOP_ProfileId;
		#ifndef _IDL_SEQUENCE_ulong_defined
			#define _IDL_SEQUENCE_ulong_defined
			SOM_SEQUENCE_TYPEDEF_NAME(unsigned long ,sequence(ulong));
		#endif /* _IDL_SEQUENCE_ulong_defined */
#ifndef IOP_TAG_INTERNET_IOP
	#define IOP_TAG_INTERNET_IOP   0UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TAG_INTERNET_IOP
			#if defined(TAG_INTERNET_IOP)
				#undef TAG_INTERNET_IOP
				#define SOMGD_TAG_INTERNET_IOP
			#else
				#define TAG_INTERNET_IOP IOP_TAG_INTERNET_IOP
			#endif
		#endif /* SOMGD_TAG_INTERNET_IOP */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TAG_INTERNET_IOP */
#ifndef IOP_TAG_MULTIPLE_COMPONENTS
	#define IOP_TAG_MULTIPLE_COMPONENTS   1UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TAG_MULTIPLE_COMPONENTS
			#if defined(TAG_MULTIPLE_COMPONENTS)
				#undef TAG_MULTIPLE_COMPONENTS
				#define SOMGD_TAG_MULTIPLE_COMPONENTS
			#else
				#define TAG_MULTIPLE_COMPONENTS IOP_TAG_MULTIPLE_COMPONENTS
			#endif
		#endif /* SOMGD_TAG_MULTIPLE_COMPONENTS */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TAG_MULTIPLE_COMPONENTS */
		typedef struct IOP_TaggedProfile
		{
			IOP_ProfileId tag;
			_IDL_SEQUENCE_octet profile_data;
		} IOP_TaggedProfile;
		#ifndef _IDL_SEQUENCE_IOP_TaggedProfile_defined
			#define _IDL_SEQUENCE_IOP_TaggedProfile_defined
			SOM_SEQUENCE_TYPEDEF(IOP_TaggedProfile);
		#endif /* _IDL_SEQUENCE_IOP_TaggedProfile_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_TaggedProfile
				#if defined(TaggedProfile)
					#undef TaggedProfile
					#define SOMGD_TaggedProfile
				#else
					#define TaggedProfile IOP_TaggedProfile
				#endif
			#endif /* SOMGD_TaggedProfile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_TaggedProfile_defined
				#define _IDL_SEQUENCE_TaggedProfile_defined
				#define _IDL_SEQUENCE_TaggedProfile _IDL_SEQUENCE_IOP_TaggedProfile
			#endif /* _IDL_SEQUENCE_TaggedProfile_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct IOP_IOR
		{
			string type_id;
			_IDL_SEQUENCE_IOP_TaggedProfile profiles;
		} IOP_IOR;
		#ifndef _IDL_SEQUENCE_IOP_IOR_defined
			#define _IDL_SEQUENCE_IOP_IOR_defined
			SOM_SEQUENCE_TYPEDEF(IOP_IOR);
		#endif /* _IDL_SEQUENCE_IOP_IOR_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_IOR
				#if defined(IOR)
					#undef IOR
					#define SOMGD_IOR
				#else
					#define IOR IOP_IOR
				#endif
			#endif /* SOMGD_IOR */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_IOR_defined
				#define _IDL_SEQUENCE_IOR_defined
				#define _IDL_SEQUENCE_IOR _IDL_SEQUENCE_IOP_IOR
			#endif /* _IDL_SEQUENCE_IOR_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef unsigned long IOP_ComponentId;
#ifndef IOP_TAG_ORB_TYPE
	#define IOP_TAG_ORB_TYPE   0UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TAG_ORB_TYPE
			#if defined(TAG_ORB_TYPE)
				#undef TAG_ORB_TYPE
				#define SOMGD_TAG_ORB_TYPE
			#else
				#define TAG_ORB_TYPE IOP_TAG_ORB_TYPE
			#endif
		#endif /* SOMGD_TAG_ORB_TYPE */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TAG_ORB_TYPE */
#ifndef IOP_TAG_CODE_SETS
	#define IOP_TAG_CODE_SETS   1UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TAG_CODE_SETS
			#if defined(TAG_CODE_SETS)
				#undef TAG_CODE_SETS
				#define SOMGD_TAG_CODE_SETS
			#else
				#define TAG_CODE_SETS IOP_TAG_CODE_SETS
			#endif
		#endif /* SOMGD_TAG_CODE_SETS */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TAG_CODE_SETS */
#ifndef IOP_TAG_SEC_NAME
	#define IOP_TAG_SEC_NAME   14UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TAG_SEC_NAME
			#if defined(TAG_SEC_NAME)
				#undef TAG_SEC_NAME
				#define SOMGD_TAG_SEC_NAME
			#else
				#define TAG_SEC_NAME IOP_TAG_SEC_NAME
			#endif
		#endif /* SOMGD_TAG_SEC_NAME */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TAG_SEC_NAME */
#ifndef IOP_TAG_ASSOCIATION_OPTIONS
	#define IOP_TAG_ASSOCIATION_OPTIONS   13UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TAG_ASSOCIATION_OPTIONS
			#if defined(TAG_ASSOCIATION_OPTIONS)
				#undef TAG_ASSOCIATION_OPTIONS
				#define SOMGD_TAG_ASSOCIATION_OPTIONS
			#else
				#define TAG_ASSOCIATION_OPTIONS IOP_TAG_ASSOCIATION_OPTIONS
			#endif
		#endif /* SOMGD_TAG_ASSOCIATION_OPTIONS */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TAG_ASSOCIATION_OPTIONS */
#ifndef IOP_TAG_GENERIC_SEC_MECH
	#define IOP_TAG_GENERIC_SEC_MECH   12UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TAG_GENERIC_SEC_MECH
			#if defined(TAG_GENERIC_SEC_MECH)
				#undef TAG_GENERIC_SEC_MECH
				#define SOMGD_TAG_GENERIC_SEC_MECH
			#else
				#define TAG_GENERIC_SEC_MECH IOP_TAG_GENERIC_SEC_MECH
			#endif
		#endif /* SOMGD_TAG_GENERIC_SEC_MECH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TAG_GENERIC_SEC_MECH */
		typedef struct IOP_TaggedComponent
		{
			IOP_ComponentId tag;
			_IDL_SEQUENCE_octet component_data;
		} IOP_TaggedComponent;
		#ifndef _IDL_SEQUENCE_IOP_TaggedComponent_defined
			#define _IDL_SEQUENCE_IOP_TaggedComponent_defined
			SOM_SEQUENCE_TYPEDEF(IOP_TaggedComponent);
		#endif /* _IDL_SEQUENCE_IOP_TaggedComponent_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_TaggedComponent
				#if defined(TaggedComponent)
					#undef TaggedComponent
					#define SOMGD_TaggedComponent
				#else
					#define TaggedComponent IOP_TaggedComponent
				#endif
			#endif /* SOMGD_TaggedComponent */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_TaggedComponent_defined
				#define _IDL_SEQUENCE_TaggedComponent_defined
				#define _IDL_SEQUENCE_TaggedComponent _IDL_SEQUENCE_IOP_TaggedComponent
			#endif /* _IDL_SEQUENCE_TaggedComponent_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef _IDL_SEQUENCE_IOP_TaggedComponent IOP_MultipleComponentProfile;
		#ifndef _IDL_SEQUENCE_IOP_MultipleComponentProfile_defined
			#define _IDL_SEQUENCE_IOP_MultipleComponentProfile_defined
			SOM_SEQUENCE_TYPEDEF(IOP_MultipleComponentProfile);
		#endif /* _IDL_SEQUENCE_IOP_MultipleComponentProfile_defined */
		typedef unsigned long IOP_ServiceId;
		typedef struct IOP_ServiceContext
		{
			IOP_ServiceId context_id;
			_IDL_SEQUENCE_octet context_data;
		} IOP_ServiceContext;
		#ifndef _IDL_SEQUENCE_IOP_ServiceContext_defined
			#define _IDL_SEQUENCE_IOP_ServiceContext_defined
			SOM_SEQUENCE_TYPEDEF(IOP_ServiceContext);
		#endif /* _IDL_SEQUENCE_IOP_ServiceContext_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_ServiceContext
				#if defined(ServiceContext)
					#undef ServiceContext
					#define SOMGD_ServiceContext
				#else
					#define ServiceContext IOP_ServiceContext
				#endif
			#endif /* SOMGD_ServiceContext */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_ServiceContext_defined
				#define _IDL_SEQUENCE_ServiceContext_defined
				#define _IDL_SEQUENCE_ServiceContext _IDL_SEQUENCE_IOP_ServiceContext
			#endif /* _IDL_SEQUENCE_ServiceContext_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef _IDL_SEQUENCE_IOP_ServiceContext IOP_ServiceContextList;
		#ifndef _IDL_SEQUENCE_IOP_ServiceContextList_defined
			#define _IDL_SEQUENCE_IOP_ServiceContextList_defined
			SOM_SEQUENCE_TYPEDEF(IOP_ServiceContextList);
		#endif /* _IDL_SEQUENCE_IOP_ServiceContextList_defined */
#ifndef IOP_TransactionService
	#define IOP_TransactionService   0UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TransactionService
			#if defined(TransactionService)
				#undef TransactionService
				#define SOMGD_TransactionService
			#else
				#define TransactionService IOP_TransactionService
			#endif
		#endif /* SOMGD_TransactionService */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_TransactionService */
#ifndef IOP_CodeSets
	#define IOP_CodeSets   1UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_CodeSets
			#if defined(CodeSets)
				#undef CodeSets
				#define SOMGD_CodeSets
			#else
				#define CodeSets IOP_CodeSets
			#endif
		#endif /* SOMGD_CodeSets */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_CodeSets */
		#ifndef _IDL_SEQUENCE_octet_defined
			#define _IDL_SEQUENCE_octet_defined
			SOM_SEQUENCE_TYPEDEF(octet);
		#endif /* _IDL_SEQUENCE_octet_defined */
		typedef _IDL_SEQUENCE_octet IOP_OctetSeq;
		#ifndef _IDL_SEQUENCE_IOP_OctetSeq_defined
			#define _IDL_SEQUENCE_IOP_OctetSeq_defined
			SOM_SEQUENCE_TYPEDEF(IOP_OctetSeq);
		#endif /* _IDL_SEQUENCE_IOP_OctetSeq_defined */
		typedef short IOP_EncodingFormat;
		#ifndef _IDL_SEQUENCE_short_defined
			#define _IDL_SEQUENCE_short_defined
			SOM_SEQUENCE_TYPEDEF(short);
		#endif /* _IDL_SEQUENCE_short_defined */
#ifndef IOP_ENCODING_CDR_ENCAPS
	#define IOP_ENCODING_CDR_ENCAPS   0
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ENCODING_CDR_ENCAPS
			#if defined(ENCODING_CDR_ENCAPS)
				#undef ENCODING_CDR_ENCAPS
				#define SOMGD_ENCODING_CDR_ENCAPS
			#else
				#define ENCODING_CDR_ENCAPS IOP_ENCODING_CDR_ENCAPS
			#endif
		#endif /* SOMGD_ENCODING_CDR_ENCAPS */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* IOP_ENCODING_CDR_ENCAPS */
		typedef struct IOP_Encoding
		{
			IOP_EncodingFormat format;
			octet major_version;
			octet minor_version;
		} IOP_Encoding;
		#ifndef _IDL_SEQUENCE_IOP_Encoding_defined
			#define _IDL_SEQUENCE_IOP_Encoding_defined
			SOM_SEQUENCE_TYPEDEF(IOP_Encoding);
		#endif /* _IDL_SEQUENCE_IOP_Encoding_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_Encoding
				#if defined(Encoding)
					#undef Encoding
					#define SOMGD_Encoding
				#else
					#define Encoding IOP_Encoding
				#endif
			#endif /* SOMGD_Encoding */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_Encoding_defined
				#define _IDL_SEQUENCE_Encoding_defined
				#define _IDL_SEQUENCE_Encoding _IDL_SEQUENCE_IOP_Encoding
			#endif /* _IDL_SEQUENCE_Encoding_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_IOP_Codec_InvalidTypeForEncoding   "::IOP::Codec::InvalidTypeForEncoding"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_InvalidTypeForEncoding
#ifndef ex_InvalidTypeForEncoding
#define ex_InvalidTypeForEncoding  ex_IOP_Codec_InvalidTypeForEncoding
#else
#define SOMTGD_ex_InvalidTypeForEncoding
#undef ex_InvalidTypeForEncoding
#endif /* ex_InvalidTypeForEncoding */
#endif /* SOMTGD_ex_InvalidTypeForEncoding */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct IOP_Codec_InvalidTypeForEncoding IOP_Codec_InvalidTypeForEncoding;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_InvalidTypeForEncoding
		#if defined(InvalidTypeForEncoding)
			#undef InvalidTypeForEncoding
			#define SOMGD_InvalidTypeForEncoding
		#else
			#define InvalidTypeForEncoding IOP_Codec_InvalidTypeForEncoding
		#endif
	#endif /* SOMGD_InvalidTypeForEncoding */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_IOP_Codec_FormatMismatch   "::IOP::Codec::FormatMismatch"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_FormatMismatch
#ifndef ex_FormatMismatch
#define ex_FormatMismatch  ex_IOP_Codec_FormatMismatch
#else
#define SOMTGD_ex_FormatMismatch
#undef ex_FormatMismatch
#endif /* ex_FormatMismatch */
#endif /* SOMTGD_ex_FormatMismatch */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct IOP_Codec_FormatMismatch IOP_Codec_FormatMismatch;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_FormatMismatch
		#if defined(FormatMismatch)
			#undef FormatMismatch
			#define SOMGD_FormatMismatch
		#else
			#define FormatMismatch IOP_Codec_FormatMismatch
		#endif
	#endif /* SOMGD_FormatMismatch */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_IOP_Codec_TypeMismatch   "::IOP::Codec::TypeMismatch"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_TypeMismatch
#ifndef ex_TypeMismatch
#define ex_TypeMismatch  ex_IOP_Codec_TypeMismatch
#else
#define SOMTGD_ex_TypeMismatch
#undef ex_TypeMismatch
#endif /* ex_TypeMismatch */
#endif /* SOMTGD_ex_TypeMismatch */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct IOP_Codec_TypeMismatch IOP_Codec_TypeMismatch;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_TypeMismatch
		#if defined(TypeMismatch)
			#undef TypeMismatch
			#define SOMGD_TypeMismatch
		#else
			#define TypeMismatch IOP_Codec_TypeMismatch
		#endif
	#endif /* SOMGD_TypeMismatch */
#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifdef __IBMC__
			typedef IOP_OctetSeq (somTP_IOP_Codec_encode)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ any *data);
			#pragma linkage(somTP_IOP_Codec_encode,system)
			typedef somTP_IOP_Codec_encode *somTD_IOP_Codec_encode;
		#else /* __IBMC__ */
			typedef IOP_OctetSeq (SOMLINK * somTD_IOP_Codec_encode)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ any *data);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef any (somTP_IOP_Codec_decode)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ IOP_OctetSeq *data);
			#pragma linkage(somTP_IOP_Codec_decode,system)
			typedef somTP_IOP_Codec_decode *somTD_IOP_Codec_decode;
		#else /* __IBMC__ */
			typedef any (SOMLINK * somTD_IOP_Codec_decode)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ IOP_OctetSeq *data);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef IOP_OctetSeq (somTP_IOP_Codec_encode_value)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ any *data);
			#pragma linkage(somTP_IOP_Codec_encode_value,system)
			typedef somTP_IOP_Codec_encode_value *somTD_IOP_Codec_encode_value;
		#else /* __IBMC__ */
			typedef IOP_OctetSeq (SOMLINK * somTD_IOP_Codec_encode_value)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ any *data);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef any (somTP_IOP_Codec_decode_value)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ IOP_OctetSeq *data,
				/* in */ TypeCode tc);
			#pragma linkage(somTP_IOP_Codec_decode_value,system)
			typedef somTP_IOP_Codec_decode_value *somTD_IOP_Codec_decode_value;
		#else /* __IBMC__ */
			typedef any (SOMLINK * somTD_IOP_Codec_decode_value)(
				IOP_Codec SOMSTAR somSelf,
				Environment *ev,
				/* in */ IOP_OctetSeq *data,
				/* in */ TypeCode tc);
		#endif /* __IBMC__ */
		#ifndef IOP_Codec_MajorVersion
			#define IOP_Codec_MajorVersion   1
		#endif /* IOP_Codec_MajorVersion */
		#ifndef IOP_Codec_MinorVersion
			#define IOP_Codec_MinorVersion   0
		#endif /* IOP_Codec_MinorVersion */
		typedef struct IOP_CodecClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken encode;
			somMToken decode;
			somMToken encode_value;
			somMToken decode_value;
			somMToken _get_encoding;
			somMToken _set_encoding;
			somMToken _get_c_orb;
			somMToken _set_c_orb;
		} IOP_CodecClassDataStructure;
		typedef struct IOP_CodecCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} IOP_CodecCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_Codec_Class_Source)
				SOMEXTERN struct IOP_CodecClassDataStructure _IOP_CodecClassData;
				#ifndef IOP_CodecClassData
					#define IOP_CodecClassData    _IOP_CodecClassData
				#endif /* IOP_CodecClassData */
			#else
				SOMEXTERN struct IOP_CodecClassDataStructure * SOMLINK resolve_IOP_CodecClassData(void);
				#ifndef IOP_CodecClassData
					#define IOP_CodecClassData    (*(resolve_IOP_CodecClassData()))
				#endif /* IOP_CodecClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_Codec_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_unotypes_Source || IOP_Codec_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_unotypes_Source || IOP_Codec_Class_Source */
			struct IOP_CodecClassDataStructure SOMDLINK IOP_CodecClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_Codec_Class_Source)
				SOMEXTERN struct IOP_CodecCClassDataStructure _IOP_CodecCClassData;
				#ifndef IOP_CodecCClassData
					#define IOP_CodecCClassData    _IOP_CodecCClassData
				#endif /* IOP_CodecCClassData */
			#else
				SOMEXTERN struct IOP_CodecCClassDataStructure * SOMLINK resolve_IOP_CodecCClassData(void);
				#ifndef IOP_CodecCClassData
					#define IOP_CodecCClassData    (*(resolve_IOP_CodecCClassData()))
				#endif /* IOP_CodecCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_Codec_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_unotypes_Source || IOP_Codec_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_unotypes_Source || IOP_Codec_Class_Source */
			struct IOP_CodecCClassDataStructure SOMDLINK IOP_CodecCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_unotypes_Source) || defined(IOP_Codec_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_unotypes_Source || IOP_Codec_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_unotypes_Source || IOP_Codec_Class_Source */
		SOMClass SOMSTAR SOMLINK IOP_CodecNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_IOP_Codec (IOP_CodecClassData.classObject)
		#ifndef SOMGD_IOP_Codec
			#if (defined(_IOP_Codec) || defined(__IOP_Codec))
				#undef _IOP_Codec
				#undef __IOP_Codec
				#define SOMGD_IOP_Codec 1
			#else
				#define _IOP_Codec _SOMCLASS_IOP_Codec
			#endif /* _IOP_Codec */
		#endif /* SOMGD_IOP_Codec */
		#define IOP_Codec_classObj _SOMCLASS_IOP_Codec
		#define _SOMMTOKEN_IOP_Codec(method) ((somMToken)(IOP_CodecClassData.method))
		#ifndef IOP_CodecNew
			#define IOP_CodecNew() ( _IOP_Codec ? \
				(SOMClass_somNew(_IOP_Codec)) : \
				( IOP_CodecNewClass( \
					IOP_Codec_MajorVersion, \
					IOP_Codec_MinorVersion), \
				SOMClass_somNew(_IOP_Codec))) 
		#endif /* NewIOP_Codec */
		#ifndef IOP_Codec_encode
			#define IOP_Codec_encode(somSelf,ev,data) \
				SOM_Resolve(somSelf,IOP_Codec,encode)  \
					(somSelf,ev,data)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__encode
					#if defined(_encode)
						#undef _encode
						#define SOMGD__encode
					#else
						#define _encode IOP_Codec_encode
					#endif
				#endif /* SOMGD__encode */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* IOP_Codec_encode */
		#ifndef IOP_Codec_decode
			#define IOP_Codec_decode(somSelf,ev,data) \
				SOM_Resolve(somSelf,IOP_Codec,decode)  \
					(somSelf,ev,data)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__decode
					#if defined(_decode)
						#undef _decode
						#define SOMGD__decode
					#else
						#define _decode IOP_Codec_decode
					#endif
				#endif /* SOMGD__decode */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* IOP_Codec_decode */
		#ifndef IOP_Codec_encode_value
			#define IOP_Codec_encode_value(somSelf,ev,data) \
				SOM_Resolve(somSelf,IOP_Codec,encode_value)  \
					(somSelf,ev,data)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__encode_value
					#if defined(_encode_value)
						#undef _encode_value
						#define SOMGD__encode_value
					#else
						#define _encode_value IOP_Codec_encode_value
					#endif
				#endif /* SOMGD__encode_value */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* IOP_Codec_encode_value */
		#ifndef IOP_Codec_decode_value
			#define IOP_Codec_decode_value(somSelf,ev,data,tc) \
				SOM_Resolve(somSelf,IOP_Codec,decode_value)  \
					(somSelf,ev,data,tc)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__decode_value
					#if defined(_decode_value)
						#undef _decode_value
						#define SOMGD__decode_value
					#else
						#define _decode_value IOP_Codec_decode_value
					#endif
				#endif /* SOMGD__decode_value */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* IOP_Codec_decode_value */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define IOP_Codec_somInit SOMObject_somInit
#define IOP_Codec_somUninit SOMObject_somUninit
#define IOP_Codec_somFree SOMObject_somFree
#define IOP_Codec_somGetClass SOMObject_somGetClass
#define IOP_Codec_somGetClassName SOMObject_somGetClassName
#define IOP_Codec_somGetSize SOMObject_somGetSize
#define IOP_Codec_somIsA SOMObject_somIsA
#define IOP_Codec_somIsInstanceOf SOMObject_somIsInstanceOf
#define IOP_Codec_somRespondsTo SOMObject_somRespondsTo
#define IOP_Codec_somDispatch SOMObject_somDispatch
#define IOP_Codec_somClassDispatch SOMObject_somClassDispatch
#define IOP_Codec_somCastObj SOMObject_somCastObj
#define IOP_Codec_somResetObj SOMObject_somResetObj
#define IOP_Codec_somPrintSelf SOMObject_somPrintSelf
#define IOP_Codec_somDumpSelf SOMObject_somDumpSelf
#define IOP_Codec_somDumpSelfInt SOMObject_somDumpSelfInt
#define IOP_Codec_somDefaultInit SOMObject_somDefaultInit
#define IOP_Codec_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define IOP_Codec_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define IOP_Codec_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define IOP_Codec_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define IOP_Codec_somDefaultAssign SOMObject_somDefaultAssign
#define IOP_Codec_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define IOP_Codec_somDefaultVAssign SOMObject_somDefaultVAssign
#define IOP_Codec_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define IOP_Codec_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#define ex_IOP_CodecFactory_UnknownEncoding   "::IOP::CodecFactory::UnknownEncoding"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_UnknownEncoding
#ifndef ex_UnknownEncoding
#define ex_UnknownEncoding  ex_IOP_CodecFactory_UnknownEncoding
#else
#define SOMTGD_ex_UnknownEncoding
#undef ex_UnknownEncoding
#endif /* ex_UnknownEncoding */
#endif /* SOMTGD_ex_UnknownEncoding */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct IOP_CodecFactory_UnknownEncoding IOP_CodecFactory_UnknownEncoding;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_UnknownEncoding
		#if defined(UnknownEncoding)
			#undef UnknownEncoding
			#define SOMGD_UnknownEncoding
		#else
			#define UnknownEncoding IOP_CodecFactory_UnknownEncoding
		#endif
	#endif /* SOMGD_UnknownEncoding */
#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifdef __IBMC__
			typedef IOP_Codec SOMSTAR (somTP_IOP_CodecFactory_create_codec)(
				IOP_CodecFactory SOMSTAR somSelf,
				Environment *ev,
				/* in */ IOP_Encoding *enc);
			#pragma linkage(somTP_IOP_CodecFactory_create_codec,system)
			typedef somTP_IOP_CodecFactory_create_codec *somTD_IOP_CodecFactory_create_codec;
		#else /* __IBMC__ */
			typedef IOP_Codec SOMSTAR (SOMLINK * somTD_IOP_CodecFactory_create_codec)(
				IOP_CodecFactory SOMSTAR somSelf,
				Environment *ev,
				/* in */ IOP_Encoding *enc);
		#endif /* __IBMC__ */
		#ifndef IOP_CodecFactory_MajorVersion
			#define IOP_CodecFactory_MajorVersion   1
		#endif /* IOP_CodecFactory_MajorVersion */
		#ifndef IOP_CodecFactory_MinorVersion
			#define IOP_CodecFactory_MinorVersion   0
		#endif /* IOP_CodecFactory_MinorVersion */
		typedef struct IOP_CodecFactoryClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken create_codec;
			somMToken _get_c_orb;
			somMToken _set_c_orb;
		} IOP_CodecFactoryClassDataStructure;
		typedef struct IOP_CodecFactoryCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} IOP_CodecFactoryCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_CodecFactory_Class_Source)
				SOMEXTERN struct IOP_CodecFactoryClassDataStructure _IOP_CodecFactoryClassData;
				#ifndef IOP_CodecFactoryClassData
					#define IOP_CodecFactoryClassData    _IOP_CodecFactoryClassData
				#endif /* IOP_CodecFactoryClassData */
			#else
				SOMEXTERN struct IOP_CodecFactoryClassDataStructure * SOMLINK resolve_IOP_CodecFactoryClassData(void);
				#ifndef IOP_CodecFactoryClassData
					#define IOP_CodecFactoryClassData    (*(resolve_IOP_CodecFactoryClassData()))
				#endif /* IOP_CodecFactoryClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_CodecFactory_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_unotypes_Source || IOP_CodecFactory_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_unotypes_Source || IOP_CodecFactory_Class_Source */
			struct IOP_CodecFactoryClassDataStructure SOMDLINK IOP_CodecFactoryClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_CodecFactory_Class_Source)
				SOMEXTERN struct IOP_CodecFactoryCClassDataStructure _IOP_CodecFactoryCClassData;
				#ifndef IOP_CodecFactoryCClassData
					#define IOP_CodecFactoryCClassData    _IOP_CodecFactoryCClassData
				#endif /* IOP_CodecFactoryCClassData */
			#else
				SOMEXTERN struct IOP_CodecFactoryCClassDataStructure * SOMLINK resolve_IOP_CodecFactoryCClassData(void);
				#ifndef IOP_CodecFactoryCClassData
					#define IOP_CodecFactoryCClassData    (*(resolve_IOP_CodecFactoryCClassData()))
				#endif /* IOP_CodecFactoryCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_unotypes_Source) || defined(IOP_CodecFactory_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_unotypes_Source || IOP_CodecFactory_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_unotypes_Source || IOP_CodecFactory_Class_Source */
			struct IOP_CodecFactoryCClassDataStructure SOMDLINK IOP_CodecFactoryCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_unotypes_Source) || defined(IOP_CodecFactory_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_unotypes_Source || IOP_CodecFactory_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_unotypes_Source || IOP_CodecFactory_Class_Source */
		SOMClass SOMSTAR SOMLINK IOP_CodecFactoryNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_IOP_CodecFactory (IOP_CodecFactoryClassData.classObject)
		#ifndef SOMGD_IOP_CodecFactory
			#if (defined(_IOP_CodecFactory) || defined(__IOP_CodecFactory))
				#undef _IOP_CodecFactory
				#undef __IOP_CodecFactory
				#define SOMGD_IOP_CodecFactory 1
			#else
				#define _IOP_CodecFactory _SOMCLASS_IOP_CodecFactory
			#endif /* _IOP_CodecFactory */
		#endif /* SOMGD_IOP_CodecFactory */
		#define IOP_CodecFactory_classObj _SOMCLASS_IOP_CodecFactory
		#define _SOMMTOKEN_IOP_CodecFactory(method) ((somMToken)(IOP_CodecFactoryClassData.method))
		#ifndef IOP_CodecFactoryNew
			#define IOP_CodecFactoryNew() ( _IOP_CodecFactory ? \
				(SOMClass_somNew(_IOP_CodecFactory)) : \
				( IOP_CodecFactoryNewClass( \
					IOP_CodecFactory_MajorVersion, \
					IOP_CodecFactory_MinorVersion), \
				SOMClass_somNew(_IOP_CodecFactory))) 
		#endif /* NewIOP_CodecFactory */
		#ifndef IOP_CodecFactory_create_codec
			#define IOP_CodecFactory_create_codec(somSelf,ev,enc) \
				SOM_Resolve(somSelf,IOP_CodecFactory,create_codec)  \
					(somSelf,ev,enc)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__create_codec
					#if defined(_create_codec)
						#undef _create_codec
						#define SOMGD__create_codec
					#else
						#define _create_codec IOP_CodecFactory_create_codec
					#endif
				#endif /* SOMGD__create_codec */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* IOP_CodecFactory_create_codec */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define IOP_CodecFactory_somInit SOMObject_somInit
#define IOP_CodecFactory_somUninit SOMObject_somUninit
#define IOP_CodecFactory_somFree SOMObject_somFree
#define IOP_CodecFactory_somGetClass SOMObject_somGetClass
#define IOP_CodecFactory_somGetClassName SOMObject_somGetClassName
#define IOP_CodecFactory_somGetSize SOMObject_somGetSize
#define IOP_CodecFactory_somIsA SOMObject_somIsA
#define IOP_CodecFactory_somIsInstanceOf SOMObject_somIsInstanceOf
#define IOP_CodecFactory_somRespondsTo SOMObject_somRespondsTo
#define IOP_CodecFactory_somDispatch SOMObject_somDispatch
#define IOP_CodecFactory_somClassDispatch SOMObject_somClassDispatch
#define IOP_CodecFactory_somCastObj SOMObject_somCastObj
#define IOP_CodecFactory_somResetObj SOMObject_somResetObj
#define IOP_CodecFactory_somPrintSelf SOMObject_somPrintSelf
#define IOP_CodecFactory_somDumpSelf SOMObject_somDumpSelf
#define IOP_CodecFactory_somDumpSelfInt SOMObject_somDumpSelfInt
#define IOP_CodecFactory_somDefaultInit SOMObject_somDefaultInit
#define IOP_CodecFactory_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define IOP_CodecFactory_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define IOP_CodecFactory_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define IOP_CodecFactory_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define IOP_CodecFactory_somDefaultAssign SOMObject_somDefaultAssign
#define IOP_CodecFactory_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define IOP_CodecFactory_somDefaultVAssign SOMObject_somDefaultVAssign
#define IOP_CodecFactory_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define IOP_CodecFactory_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef GIOP_MsgType_defined
			#define GIOP_MsgType_defined
			typedef unsigned long GIOP_MsgType;
			#ifndef GIOP_Request
				#define GIOP_Request 1UL
			#endif /* GIOP_Request */
			#ifndef GIOP_Reply
				#define GIOP_Reply 2UL
			#endif /* GIOP_Reply */
			#ifndef GIOP_CancelRequest
				#define GIOP_CancelRequest 3UL
			#endif /* GIOP_CancelRequest */
			#ifndef GIOP_LocateRequest
				#define GIOP_LocateRequest 4UL
			#endif /* GIOP_LocateRequest */
			#ifndef GIOP_LocateReply
				#define GIOP_LocateReply 5UL
			#endif /* GIOP_LocateReply */
			#ifndef GIOP_CloseConnection
				#define GIOP_CloseConnection 6UL
			#endif /* GIOP_CloseConnection */
			#ifndef GIOP_MessageError
				#define GIOP_MessageError 7UL
			#endif /* GIOP_MessageError */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_MsgType
					#if defined(MsgType)
						#undef MsgType
						#define SOMGD_MsgType
					#else
						#define MsgType GIOP_MsgType
					#endif
				#endif /* SOMGD_MsgType */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* GIOP_MsgType_defined */
		typedef struct GIOP_Version
		{
			char major;
			char minor;
		} GIOP_Version;
		#ifndef _IDL_SEQUENCE_GIOP_Version_defined
			#define _IDL_SEQUENCE_GIOP_Version_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_Version);
		#endif /* _IDL_SEQUENCE_GIOP_Version_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_Version
				#if defined(Version)
					#undef Version
					#define SOMGD_Version
				#else
					#define Version GIOP_Version
				#endif
			#endif /* SOMGD_Version */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_Version_defined
				#define _IDL_SEQUENCE_Version_defined
				#define _IDL_SEQUENCE_Version _IDL_SEQUENCE_GIOP_Version
			#endif /* _IDL_SEQUENCE_Version_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct GIOP_MessageHeader
		{
			char magic[4];
			GIOP_Version protocol_version;
			octet flags;
			octet message_type;
			unsigned long message_size;
		} GIOP_MessageHeader;
		#ifndef _IDL_SEQUENCE_GIOP_MessageHeader_defined
			#define _IDL_SEQUENCE_GIOP_MessageHeader_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_MessageHeader);
		#endif /* _IDL_SEQUENCE_GIOP_MessageHeader_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_MessageHeader
				#if defined(MessageHeader)
					#undef MessageHeader
					#define SOMGD_MessageHeader
				#else
					#define MessageHeader GIOP_MessageHeader
				#endif
			#endif /* SOMGD_MessageHeader */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_MessageHeader_defined
				#define _IDL_SEQUENCE_MessageHeader_defined
				#define _IDL_SEQUENCE_MessageHeader _IDL_SEQUENCE_GIOP_MessageHeader
			#endif /* _IDL_SEQUENCE_MessageHeader_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct GIOP_RequestHeader_1_0
		{
			IOP_ServiceContextList service_context;
			unsigned long request_id;
			boolean response_expected;
			_IDL_SEQUENCE_octet object_key;
			string operation;
			Principal SOMSTAR requesting_principal;
		} GIOP_RequestHeader_1_0;
		#ifndef _IDL_SEQUENCE_GIOP_RequestHeader_1_0_defined
			#define _IDL_SEQUENCE_GIOP_RequestHeader_1_0_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_RequestHeader_1_0);
		#endif /* _IDL_SEQUENCE_GIOP_RequestHeader_1_0_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_RequestHeader_1_0
				#if defined(RequestHeader_1_0)
					#undef RequestHeader_1_0
					#define SOMGD_RequestHeader_1_0
				#else
					#define RequestHeader_1_0 GIOP_RequestHeader_1_0
				#endif
			#endif /* SOMGD_RequestHeader_1_0 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_RequestHeader_1_0_defined
				#define _IDL_SEQUENCE_RequestHeader_1_0_defined
				#define _IDL_SEQUENCE_RequestHeader_1_0 _IDL_SEQUENCE_GIOP_RequestHeader_1_0
			#endif /* _IDL_SEQUENCE_RequestHeader_1_0_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef short GIOP_AddressingDisposition;
#ifndef GIOP_KeyAddr
	#define GIOP_KeyAddr   0
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_KeyAddr
			#if defined(KeyAddr)
				#undef KeyAddr
				#define SOMGD_KeyAddr
			#else
				#define KeyAddr GIOP_KeyAddr
			#endif
		#endif /* SOMGD_KeyAddr */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* GIOP_KeyAddr */
#ifndef GIOP_ProfileAddr
	#define GIOP_ProfileAddr   1
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ProfileAddr
			#if defined(ProfileAddr)
				#undef ProfileAddr
				#define SOMGD_ProfileAddr
			#else
				#define ProfileAddr GIOP_ProfileAddr
			#endif
		#endif /* SOMGD_ProfileAddr */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* GIOP_ProfileAddr */
#ifndef GIOP_ReferenceAddr
	#define GIOP_ReferenceAddr   2
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ReferenceAddr
			#if defined(ReferenceAddr)
				#undef ReferenceAddr
				#define SOMGD_ReferenceAddr
			#else
				#define ReferenceAddr GIOP_ReferenceAddr
			#endif
		#endif /* SOMGD_ReferenceAddr */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* GIOP_ReferenceAddr */
		typedef struct GIOP_IORAddressingInfo
		{
			unsigned long selected_profile_index;
			IOP_IOR ior;
		} GIOP_IORAddressingInfo;
		#ifndef _IDL_SEQUENCE_GIOP_IORAddressingInfo_defined
			#define _IDL_SEQUENCE_GIOP_IORAddressingInfo_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_IORAddressingInfo);
		#endif /* _IDL_SEQUENCE_GIOP_IORAddressingInfo_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_IORAddressingInfo
				#if defined(IORAddressingInfo)
					#undef IORAddressingInfo
					#define SOMGD_IORAddressingInfo
				#else
					#define IORAddressingInfo GIOP_IORAddressingInfo
				#endif
			#endif /* SOMGD_IORAddressingInfo */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_IORAddressingInfo_defined
				#define _IDL_SEQUENCE_IORAddressingInfo_defined
				#define _IDL_SEQUENCE_IORAddressingInfo _IDL_SEQUENCE_GIOP_IORAddressingInfo
			#endif /* _IDL_SEQUENCE_IORAddressingInfo_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct GIOP_TargetAddress 
{
	GIOP_AddressingDisposition _d;
	union
	{
		_IDL_SEQUENCE_octet object_key;
		IOP_TaggedProfile profile;
		GIOP_IORAddressingInfo ior;
	} _u;
} GIOP_TargetAddress;
		typedef struct GIOP_RequestHeader_1_2
		{
			unsigned long request_id;
			octet response_flags;
			octet reserved[3];
			GIOP_TargetAddress target;
			string operation;
			IOP_ServiceContextList service_context;
		} GIOP_RequestHeader_1_2;
		#ifndef _IDL_SEQUENCE_GIOP_RequestHeader_1_2_defined
			#define _IDL_SEQUENCE_GIOP_RequestHeader_1_2_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_RequestHeader_1_2);
		#endif /* _IDL_SEQUENCE_GIOP_RequestHeader_1_2_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_RequestHeader_1_2
				#if defined(RequestHeader_1_2)
					#undef RequestHeader_1_2
					#define SOMGD_RequestHeader_1_2
				#else
					#define RequestHeader_1_2 GIOP_RequestHeader_1_2
				#endif
			#endif /* SOMGD_RequestHeader_1_2 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_RequestHeader_1_2_defined
				#define _IDL_SEQUENCE_RequestHeader_1_2_defined
				#define _IDL_SEQUENCE_RequestHeader_1_2 _IDL_SEQUENCE_GIOP_RequestHeader_1_2
			#endif /* _IDL_SEQUENCE_RequestHeader_1_2_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef GIOP_ReplyStatusType_1_0_defined
			#define GIOP_ReplyStatusType_1_0_defined
			typedef unsigned long GIOP_ReplyStatusType_1_0;
			#ifndef GIOP_NO_EXCEPTION_1_0
				#define GIOP_NO_EXCEPTION_1_0 1UL
			#endif /* GIOP_NO_EXCEPTION_1_0 */
			#ifndef GIOP_USER_EXCEPTION_1_0
				#define GIOP_USER_EXCEPTION_1_0 2UL
			#endif /* GIOP_USER_EXCEPTION_1_0 */
			#ifndef GIOP_SYSTEM_EXCEPTION_1_0
				#define GIOP_SYSTEM_EXCEPTION_1_0 3UL
			#endif /* GIOP_SYSTEM_EXCEPTION_1_0 */
			#ifndef GIOP_LOCATION_FORWARD_1_0
				#define GIOP_LOCATION_FORWARD_1_0 4UL
			#endif /* GIOP_LOCATION_FORWARD_1_0 */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_ReplyStatusType_1_0
					#if defined(ReplyStatusType_1_0)
						#undef ReplyStatusType_1_0
						#define SOMGD_ReplyStatusType_1_0
					#else
						#define ReplyStatusType_1_0 GIOP_ReplyStatusType_1_0
					#endif
				#endif /* SOMGD_ReplyStatusType_1_0 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* GIOP_ReplyStatusType_1_0_defined */
		#ifndef GIOP_ReplyStatusType_1_2_defined
			#define GIOP_ReplyStatusType_1_2_defined
			typedef unsigned long GIOP_ReplyStatusType_1_2;
			#ifndef GIOP_NO_EXCEPTION
				#define GIOP_NO_EXCEPTION 1UL
			#endif /* GIOP_NO_EXCEPTION */
			#ifndef GIOP_USER_EXCEPTION
				#define GIOP_USER_EXCEPTION 2UL
			#endif /* GIOP_USER_EXCEPTION */
			#ifndef GIOP_SYSTEM_EXCEPTION
				#define GIOP_SYSTEM_EXCEPTION 3UL
			#endif /* GIOP_SYSTEM_EXCEPTION */
			#ifndef GIOP_LOCATION_FORWARD
				#define GIOP_LOCATION_FORWARD 4UL
			#endif /* GIOP_LOCATION_FORWARD */
			#ifndef GIOP_LOCATION_FORWARD_PERM
				#define GIOP_LOCATION_FORWARD_PERM 5UL
			#endif /* GIOP_LOCATION_FORWARD_PERM */
			#ifndef GIOP_NEEDS_ADDRESSING_MODE
				#define GIOP_NEEDS_ADDRESSING_MODE 6UL
			#endif /* GIOP_NEEDS_ADDRESSING_MODE */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_ReplyStatusType_1_2
					#if defined(ReplyStatusType_1_2)
						#undef ReplyStatusType_1_2
						#define SOMGD_ReplyStatusType_1_2
					#else
						#define ReplyStatusType_1_2 GIOP_ReplyStatusType_1_2
					#endif
				#endif /* SOMGD_ReplyStatusType_1_2 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* GIOP_ReplyStatusType_1_2_defined */
		typedef struct GIOP_ReplyHeader_1_0
		{
			IOP_ServiceContextList service_context;
			unsigned long request_id;
			GIOP_ReplyStatusType_1_0 reply_status;
		} GIOP_ReplyHeader_1_0;
		#ifndef _IDL_SEQUENCE_GIOP_ReplyHeader_1_0_defined
			#define _IDL_SEQUENCE_GIOP_ReplyHeader_1_0_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_ReplyHeader_1_0);
		#endif /* _IDL_SEQUENCE_GIOP_ReplyHeader_1_0_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_ReplyHeader_1_0
				#if defined(ReplyHeader_1_0)
					#undef ReplyHeader_1_0
					#define SOMGD_ReplyHeader_1_0
				#else
					#define ReplyHeader_1_0 GIOP_ReplyHeader_1_0
				#endif
			#endif /* SOMGD_ReplyHeader_1_0 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_ReplyHeader_1_0_defined
				#define _IDL_SEQUENCE_ReplyHeader_1_0_defined
				#define _IDL_SEQUENCE_ReplyHeader_1_0 _IDL_SEQUENCE_GIOP_ReplyHeader_1_0
			#endif /* _IDL_SEQUENCE_ReplyHeader_1_0_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct GIOP_ReplyHeader_1_2
		{
			unsigned long request_id;
			GIOP_ReplyStatusType_1_2 reply_status;
			IOP_ServiceContextList service_context;
		} GIOP_ReplyHeader_1_2;
		#ifndef _IDL_SEQUENCE_GIOP_ReplyHeader_1_2_defined
			#define _IDL_SEQUENCE_GIOP_ReplyHeader_1_2_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_ReplyHeader_1_2);
		#endif /* _IDL_SEQUENCE_GIOP_ReplyHeader_1_2_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_ReplyHeader_1_2
				#if defined(ReplyHeader_1_2)
					#undef ReplyHeader_1_2
					#define SOMGD_ReplyHeader_1_2
				#else
					#define ReplyHeader_1_2 GIOP_ReplyHeader_1_2
				#endif
			#endif /* SOMGD_ReplyHeader_1_2 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_ReplyHeader_1_2_defined
				#define _IDL_SEQUENCE_ReplyHeader_1_2_defined
				#define _IDL_SEQUENCE_ReplyHeader_1_2 _IDL_SEQUENCE_GIOP_ReplyHeader_1_2
			#endif /* _IDL_SEQUENCE_ReplyHeader_1_2_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct GIOP_CancelRequestHeader
		{
			unsigned long request_id;
		} GIOP_CancelRequestHeader;
		#ifndef _IDL_SEQUENCE_GIOP_CancelRequestHeader_defined
			#define _IDL_SEQUENCE_GIOP_CancelRequestHeader_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_CancelRequestHeader);
		#endif /* _IDL_SEQUENCE_GIOP_CancelRequestHeader_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_CancelRequestHeader
				#if defined(CancelRequestHeader)
					#undef CancelRequestHeader
					#define SOMGD_CancelRequestHeader
				#else
					#define CancelRequestHeader GIOP_CancelRequestHeader
				#endif
			#endif /* SOMGD_CancelRequestHeader */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_CancelRequestHeader_defined
				#define _IDL_SEQUENCE_CancelRequestHeader_defined
				#define _IDL_SEQUENCE_CancelRequestHeader _IDL_SEQUENCE_GIOP_CancelRequestHeader
			#endif /* _IDL_SEQUENCE_CancelRequestHeader_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct GIOP_LocateRequestHeader
		{
			unsigned long request_id;
			_IDL_SEQUENCE_octet object_key;
		} GIOP_LocateRequestHeader;
		#ifndef _IDL_SEQUENCE_GIOP_LocateRequestHeader_defined
			#define _IDL_SEQUENCE_GIOP_LocateRequestHeader_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_LocateRequestHeader);
		#endif /* _IDL_SEQUENCE_GIOP_LocateRequestHeader_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_LocateRequestHeader
				#if defined(LocateRequestHeader)
					#undef LocateRequestHeader
					#define SOMGD_LocateRequestHeader
				#else
					#define LocateRequestHeader GIOP_LocateRequestHeader
				#endif
			#endif /* SOMGD_LocateRequestHeader */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_LocateRequestHeader_defined
				#define _IDL_SEQUENCE_LocateRequestHeader_defined
				#define _IDL_SEQUENCE_LocateRequestHeader _IDL_SEQUENCE_GIOP_LocateRequestHeader
			#endif /* _IDL_SEQUENCE_LocateRequestHeader_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef GIOP_LocateStatusType_1_0_defined
			#define GIOP_LocateStatusType_1_0_defined
			typedef unsigned long GIOP_LocateStatusType_1_0;
			#ifndef GIOP_UNKNOWN_OBJECT_1_0
				#define GIOP_UNKNOWN_OBJECT_1_0 1UL
			#endif /* GIOP_UNKNOWN_OBJECT_1_0 */
			#ifndef GIOP_OBJECT_HERE_1_0
				#define GIOP_OBJECT_HERE_1_0 2UL
			#endif /* GIOP_OBJECT_HERE_1_0 */
			#ifndef GIOP_OBJECT_FORWARD_1_0
				#define GIOP_OBJECT_FORWARD_1_0 3UL
			#endif /* GIOP_OBJECT_FORWARD_1_0 */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_LocateStatusType_1_0
					#if defined(LocateStatusType_1_0)
						#undef LocateStatusType_1_0
						#define SOMGD_LocateStatusType_1_0
					#else
						#define LocateStatusType_1_0 GIOP_LocateStatusType_1_0
					#endif
				#endif /* SOMGD_LocateStatusType_1_0 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* GIOP_LocateStatusType_1_0_defined */
		#ifndef GIOP_LocateStatusType_1_2_defined
			#define GIOP_LocateStatusType_1_2_defined
			typedef unsigned long GIOP_LocateStatusType_1_2;
			#ifndef GIOP_UNKNOWN_OBJECT
				#define GIOP_UNKNOWN_OBJECT 1UL
			#endif /* GIOP_UNKNOWN_OBJECT */
			#ifndef GIOP_OBJECT_HERE
				#define GIOP_OBJECT_HERE 2UL
			#endif /* GIOP_OBJECT_HERE */
			#ifndef GIOP_OBJECT_FORWARD
				#define GIOP_OBJECT_FORWARD 3UL
			#endif /* GIOP_OBJECT_FORWARD */
			#ifndef GIOP_OBJECT_FORWARD_PERM
				#define GIOP_OBJECT_FORWARD_PERM 4UL
			#endif /* GIOP_OBJECT_FORWARD_PERM */
			#ifndef GIOP_LOC_SYSTEM_EXCEPTION
				#define GIOP_LOC_SYSTEM_EXCEPTION 5UL
			#endif /* GIOP_LOC_SYSTEM_EXCEPTION */
			#ifndef GIOP_LOC_NEEDS_ADDRESSING_MODE
				#define GIOP_LOC_NEEDS_ADDRESSING_MODE 6UL
			#endif /* GIOP_LOC_NEEDS_ADDRESSING_MODE */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_LocateStatusType_1_2
					#if defined(LocateStatusType_1_2)
						#undef LocateStatusType_1_2
						#define SOMGD_LocateStatusType_1_2
					#else
						#define LocateStatusType_1_2 GIOP_LocateStatusType_1_2
					#endif
				#endif /* SOMGD_LocateStatusType_1_2 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* GIOP_LocateStatusType_1_2_defined */
		typedef struct GIOP_LocateReplyHeader_1_0
		{
			unsigned long request_id;
			GIOP_LocateStatusType_1_0 locate_status;
		} GIOP_LocateReplyHeader_1_0;
		#ifndef _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_0_defined
			#define _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_0_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_LocateReplyHeader_1_0);
		#endif /* _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_0_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_LocateReplyHeader_1_0
				#if defined(LocateReplyHeader_1_0)
					#undef LocateReplyHeader_1_0
					#define SOMGD_LocateReplyHeader_1_0
				#else
					#define LocateReplyHeader_1_0 GIOP_LocateReplyHeader_1_0
				#endif
			#endif /* SOMGD_LocateReplyHeader_1_0 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_LocateReplyHeader_1_0_defined
				#define _IDL_SEQUENCE_LocateReplyHeader_1_0_defined
				#define _IDL_SEQUENCE_LocateReplyHeader_1_0 _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_0
			#endif /* _IDL_SEQUENCE_LocateReplyHeader_1_0_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct GIOP_LocateReplyHeader_1_2
		{
			unsigned long request_id;
			GIOP_LocateStatusType_1_2 locate_status;
		} GIOP_LocateReplyHeader_1_2;
		#ifndef _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_2_defined
			#define _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_2_defined
			SOM_SEQUENCE_TYPEDEF(GIOP_LocateReplyHeader_1_2);
		#endif /* _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_2_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_LocateReplyHeader_1_2
				#if defined(LocateReplyHeader_1_2)
					#undef LocateReplyHeader_1_2
					#define SOMGD_LocateReplyHeader_1_2
				#else
					#define LocateReplyHeader_1_2 GIOP_LocateReplyHeader_1_2
				#endif
			#endif /* SOMGD_LocateReplyHeader_1_2 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_LocateReplyHeader_1_2_defined
				#define _IDL_SEQUENCE_LocateReplyHeader_1_2_defined
				#define _IDL_SEQUENCE_LocateReplyHeader_1_2 _IDL_SEQUENCE_GIOP_LocateReplyHeader_1_2
			#endif /* _IDL_SEQUENCE_LocateReplyHeader_1_2_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct IIOP_Version
		{
			char major;
			char minor;
		} IIOP_Version;
		#ifndef _IDL_SEQUENCE_IIOP_Version_defined
			#define _IDL_SEQUENCE_IIOP_Version_defined
			SOM_SEQUENCE_TYPEDEF(IIOP_Version);
		#endif /* _IDL_SEQUENCE_IIOP_Version_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_Version
				#if defined(Version)
					#undef Version
					#define SOMGD_Version
				#else
					#define Version IIOP_Version
				#endif
			#endif /* SOMGD_Version */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_Version_defined
				#define _IDL_SEQUENCE_Version_defined
				#define _IDL_SEQUENCE_Version _IDL_SEQUENCE_IIOP_Version
			#endif /* _IDL_SEQUENCE_Version_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct IIOP_ProfileBody_1_0
		{
			IIOP_Version iiop_version;
			string host;
			unsigned short port;
			_IDL_SEQUENCE_octet object_key;
		} IIOP_ProfileBody_1_0;
		#ifndef _IDL_SEQUENCE_IIOP_ProfileBody_1_0_defined
			#define _IDL_SEQUENCE_IIOP_ProfileBody_1_0_defined
			SOM_SEQUENCE_TYPEDEF(IIOP_ProfileBody_1_0);
		#endif /* _IDL_SEQUENCE_IIOP_ProfileBody_1_0_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_ProfileBody_1_0
				#if defined(ProfileBody_1_0)
					#undef ProfileBody_1_0
					#define SOMGD_ProfileBody_1_0
				#else
					#define ProfileBody_1_0 IIOP_ProfileBody_1_0
				#endif
			#endif /* SOMGD_ProfileBody_1_0 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_ProfileBody_1_0_defined
				#define _IDL_SEQUENCE_ProfileBody_1_0_defined
				#define _IDL_SEQUENCE_ProfileBody_1_0 _IDL_SEQUENCE_IIOP_ProfileBody_1_0
			#endif /* _IDL_SEQUENCE_ProfileBody_1_0_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct IIOP_ProfileBody_1_1
		{
			IIOP_Version iiop_version;
			string host;
			unsigned short port;
			_IDL_SEQUENCE_octet object_key;
			_IDL_SEQUENCE_IOP_TaggedComponent components;
		} IIOP_ProfileBody_1_1;
		#ifndef _IDL_SEQUENCE_IIOP_ProfileBody_1_1_defined
			#define _IDL_SEQUENCE_IIOP_ProfileBody_1_1_defined
			SOM_SEQUENCE_TYPEDEF(IIOP_ProfileBody_1_1);
		#endif /* _IDL_SEQUENCE_IIOP_ProfileBody_1_1_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_ProfileBody_1_1
				#if defined(ProfileBody_1_1)
					#undef ProfileBody_1_1
					#define SOMGD_ProfileBody_1_1
				#else
					#define ProfileBody_1_1 IIOP_ProfileBody_1_1
				#endif
			#endif /* SOMGD_ProfileBody_1_1 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_ProfileBody_1_1_defined
				#define _IDL_SEQUENCE_ProfileBody_1_1_defined
				#define _IDL_SEQUENCE_ProfileBody_1_1 _IDL_SEQUENCE_IIOP_ProfileBody_1_1
			#endif /* _IDL_SEQUENCE_ProfileBody_1_1_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef struct IIOP_SOMDServiceContext
		{
			IOP_ServiceId context_tag;
			_IDL_SEQUENCE_octet context_data;
			somToken context_body;
		} IIOP_SOMDServiceContext;
		#ifndef _IDL_SEQUENCE_IIOP_SOMDServiceContext_defined
			#define _IDL_SEQUENCE_IIOP_SOMDServiceContext_defined
			SOM_SEQUENCE_TYPEDEF(IIOP_SOMDServiceContext);
		#endif /* _IDL_SEQUENCE_IIOP_SOMDServiceContext_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_SOMDServiceContext
				#if defined(SOMDServiceContext)
					#undef SOMDServiceContext
					#define SOMGD_SOMDServiceContext
				#else
					#define SOMDServiceContext IIOP_SOMDServiceContext
				#endif
			#endif /* SOMGD_SOMDServiceContext */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_SOMDServiceContext_defined
				#define _IDL_SEQUENCE_SOMDServiceContext_defined
				#define _IDL_SEQUENCE_SOMDServiceContext _IDL_SEQUENCE_IIOP_SOMDServiceContext
			#endif /* _IDL_SEQUENCE_SOMDServiceContext_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef IIOP_SOMDServiceContext IIOP_SOMDServiceContexts;
		typedef _IDL_SEQUENCE_IIOP_SOMDServiceContext IIOP_SOMDServiceContextList;
		#ifndef _IDL_SEQUENCE_IIOP_SOMDServiceContextList_defined
			#define _IDL_SEQUENCE_IIOP_SOMDServiceContextList_defined
			SOM_SEQUENCE_TYPEDEF(IIOP_SOMDServiceContextList);
		#endif /* _IDL_SEQUENCE_IIOP_SOMDServiceContextList_defined */
#endif /* SOM_Module_unotypes_Header_h */
