/* generated from somdtype.idl */
/* internal conditional is SOM_Module_somdtype_Source */
#ifndef SOM_Module_somdtype_Header_h
	#define SOM_Module_somdtype_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <containd.h>
	typedef unsigned long ORBStatus;
	#ifndef _IDL_SEQUENCE_ulong_defined
		#define _IDL_SEQUENCE_ulong_defined
		SOM_SEQUENCE_TYPEDEF_NAME(unsigned long ,sequence(ulong));
	#endif /* _IDL_SEQUENCE_ulong_defined */
	typedef unsigned long Flags;
	typedef struct NamedValue
	{
		Identifier name;
		any argument;
		long len;
		Flags arg_modes;
	} NamedValue;
	#ifndef _IDL_SEQUENCE_NamedValue_defined
		#define _IDL_SEQUENCE_NamedValue_defined
		SOM_SEQUENCE_TYPEDEF(NamedValue);
	#endif /* _IDL_SEQUENCE_NamedValue_defined */
	#ifndef _IDL_SEQUENCE_octet_defined
		#define _IDL_SEQUENCE_octet_defined
		SOM_SEQUENCE_TYPEDEF(octet);
	#endif /* _IDL_SEQUENCE_octet_defined */
	typedef _IDL_SEQUENCE_octet ReferenceData;
	#ifndef _IDL_SEQUENCE_ReferenceData_defined
		#define _IDL_SEQUENCE_ReferenceData_defined
		SOM_SEQUENCE_TYPEDEF(ReferenceData);
	#endif /* _IDL_SEQUENCE_ReferenceData_defined */
	typedef string ORBid;
	#ifndef _IDL_SEQUENCE_string_defined
		#define _IDL_SEQUENCE_string_defined
		SOM_SEQUENCE_TYPEDEF(string);
	#endif /* _IDL_SEQUENCE_string_defined */
	typedef string ImplId;
		#ifndef SOMD_marshaling_direction_t_defined
			#define SOMD_marshaling_direction_t_defined
			typedef unsigned long SOMD_marshaling_direction_t;
			#ifndef SOMD_DirCall
				#define SOMD_DirCall 1UL
			#endif /* SOMD_DirCall */
			#ifndef SOMD_DirReply
				#define SOMD_DirReply 2UL
			#endif /* SOMD_DirReply */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_marshaling_direction_t
					#if defined(marshaling_direction_t)
						#undef marshaling_direction_t
						#define SOMGD_marshaling_direction_t
					#else
						#define marshaling_direction_t SOMD_marshaling_direction_t
					#endif
				#endif /* SOMGD_marshaling_direction_t */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* SOMD_marshaling_direction_t_defined */
		#ifndef SOMD_marshaling_op_t_defined
			#define SOMD_marshaling_op_t_defined
			typedef unsigned long SOMD_marshaling_op_t;
			#ifndef SOMD_OpMarshal
				#define SOMD_OpMarshal 1UL
			#endif /* SOMD_OpMarshal */
			#ifndef SOMD_OpDemarshal
				#define SOMD_OpDemarshal 2UL
			#endif /* SOMD_OpDemarshal */
			#ifndef SOMD_OpFreeStorage
				#define SOMD_OpFreeStorage 3UL
			#endif /* SOMD_OpFreeStorage */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_marshaling_op_t
					#if defined(marshaling_op_t)
						#undef marshaling_op_t
						#define SOMGD_marshaling_op_t
					#else
						#define marshaling_op_t SOMD_marshaling_op_t
					#endif
				#endif /* SOMGD_marshaling_op_t */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* SOMD_marshaling_op_t_defined */
		typedef struct SOMD_ImplEntry
		{
			long ImplDataSize;
			char *ImplData;
		} SOMD_ImplEntry;
		#ifndef _IDL_SEQUENCE_SOMD_ImplEntry_defined
			#define _IDL_SEQUENCE_SOMD_ImplEntry_defined
			SOM_SEQUENCE_TYPEDEF(SOMD_ImplEntry);
		#endif /* _IDL_SEQUENCE_SOMD_ImplEntry_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_ImplEntry
				#if defined(ImplEntry)
					#undef ImplEntry
					#define SOMGD_ImplEntry
				#else
					#define ImplEntry SOMD_ImplEntry
				#endif
			#endif /* SOMGD_ImplEntry */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_ImplEntry_defined
				#define _IDL_SEQUENCE_ImplEntry_defined
				#define _IDL_SEQUENCE_ImplEntry _IDL_SEQUENCE_SOMD_ImplEntry
			#endif /* _IDL_SEQUENCE_ImplEntry_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* SOM_Module_somdtype_Header_h */
