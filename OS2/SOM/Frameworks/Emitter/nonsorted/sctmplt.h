/* generated from sctmplt.idl */
/* internal conditional is SOM_Module_sctmplt_Source */
#ifndef SOM_Module_sctmplt_Header_h
	#define SOM_Module_sctmplt_Header_h 1
#include <stdio.h>
typedef enum somtCommentStyleT { somtDashesE, somtCPPE, somtCSimpleE, somtCBlockE } somtCommentStyleT;
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_SOMStringTableC_defined
		#define _IDL_SOMStringTableC_defined
		typedef SOMObject SOMStringTableC;
	#endif /* _IDL_SOMStringTableC_defined */
	#ifndef _IDL_SEQUENCE_SOMStringTableC_defined
		#define _IDL_SEQUENCE_SOMStringTableC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMStringTableC SOMSTAR ,sequence(SOMStringTableC));
	#endif /* _IDL_SEQUENCE_SOMStringTableC_defined */
	#ifndef _IDL_SOMTTemplateOutputC_defined
		#define _IDL_SOMTTemplateOutputC_defined
		typedef SOMObject SOMTTemplateOutputC;
	#endif /* _IDL_SOMTTemplateOutputC_defined */
	#ifndef _IDL_SEQUENCE_SOMTTemplateOutputC_defined
		#define _IDL_SEQUENCE_SOMTTemplateOutputC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTTemplateOutputC SOMSTAR ,sequence(SOMTTemplateOutputC));
	#endif /* _IDL_SEQUENCE_SOMTTemplateOutputC_defined */
#ifndef SOMTTemplateOutputC_MAX_INPUT_LINE_LENGTH
	#define SOMTTemplateOutputC_MAX_INPUT_LINE_LENGTH   1024L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_MAX_INPUT_LINE_LENGTH
			#if defined(MAX_INPUT_LINE_LENGTH)
				#undef MAX_INPUT_LINE_LENGTH
				#define SOMGD_MAX_INPUT_LINE_LENGTH
			#else
				#define MAX_INPUT_LINE_LENGTH SOMTTemplateOutputC_MAX_INPUT_LINE_LENGTH
			#endif
		#endif /* SOMGD_MAX_INPUT_LINE_LENGTH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* SOMTTemplateOutputC_MAX_INPUT_LINE_LENGTH */
#ifndef SOMTTemplateOutputC_MAX_OUTPUT_LINE_LENGTH
	#define SOMTTemplateOutputC_MAX_OUTPUT_LINE_LENGTH   4096L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_MAX_OUTPUT_LINE_LENGTH
			#if defined(MAX_OUTPUT_LINE_LENGTH)
				#undef MAX_OUTPUT_LINE_LENGTH
				#define SOMGD_MAX_OUTPUT_LINE_LENGTH
			#else
				#define MAX_OUTPUT_LINE_LENGTH SOMTTemplateOutputC_MAX_OUTPUT_LINE_LENGTH
			#endif
		#endif /* SOMGD_MAX_OUTPUT_LINE_LENGTH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* SOMTTemplateOutputC_MAX_OUTPUT_LINE_LENGTH */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC__set_somtCommentStyle)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ somtCommentStyleT somtCommentStyle);
		#pragma linkage(somTP_SOMTTemplateOutputC__set_somtCommentStyle,system)
		typedef somTP_SOMTTemplateOutputC__set_somtCommentStyle *somTD_SOMTTemplateOutputC__set_somtCommentStyle;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC__set_somtCommentStyle)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ somtCommentStyleT somtCommentStyle);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somtCommentStyleT (somTP_SOMTTemplateOutputC__get_somtCommentStyle)(
			SOMTTemplateOutputC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTTemplateOutputC__get_somtCommentStyle,system)
		typedef somTP_SOMTTemplateOutputC__get_somtCommentStyle *somTD_SOMTTemplateOutputC__get_somtCommentStyle;
	#else /* __IBMC__ */
		typedef somtCommentStyleT (SOMLINK * somTD_SOMTTemplateOutputC__get_somtCommentStyle)(
			SOMTTemplateOutputC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC__set_somtLineLength)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ long somtLineLength);
		#pragma linkage(somTP_SOMTTemplateOutputC__set_somtLineLength,system)
		typedef somTP_SOMTTemplateOutputC__set_somtLineLength *somTD_SOMTTemplateOutputC__set_somtLineLength;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC__set_somtLineLength)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ long somtLineLength);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMTTemplateOutputC__get_somtLineLength)(
			SOMTTemplateOutputC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTTemplateOutputC__get_somtLineLength,system)
		typedef somTP_SOMTTemplateOutputC__get_somtLineLength *somTD_SOMTTemplateOutputC__get_somtLineLength;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMTTemplateOutputC__get_somtLineLength)(
			SOMTTemplateOutputC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC__set_somtCommentNewline)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ boolean somtCommentNewline);
		#pragma linkage(somTP_SOMTTemplateOutputC__set_somtCommentNewline,system)
		typedef somTP_SOMTTemplateOutputC__set_somtCommentNewline *somTD_SOMTTemplateOutputC__set_somtCommentNewline;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC__set_somtCommentNewline)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ boolean somtCommentNewline);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTTemplateOutputC__get_somtCommentNewline)(
			SOMTTemplateOutputC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTTemplateOutputC__get_somtCommentNewline,system)
		typedef somTP_SOMTTemplateOutputC__get_somtCommentNewline *somTD_SOMTTemplateOutputC__get_somtCommentNewline;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTTemplateOutputC__get_somtCommentNewline)(
			SOMTTemplateOutputC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTTemplateOutputC_somtGetSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtGetSymbol,system)
		typedef somTP_SOMTTemplateOutputC_somtGetSymbol *somTD_SOMTTemplateOutputC_somtGetSymbol;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTTemplateOutputC_somtGetSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtSetSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtSetSymbol,system)
		typedef somTP_SOMTTemplateOutputC_somtSetSymbol *somTD_SOMTTemplateOutputC_somtSetSymbol;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtSetSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtSetSymbolCopyName)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtSetSymbolCopyName,system)
		typedef somTP_SOMTTemplateOutputC_somtSetSymbolCopyName *somTD_SOMTTemplateOutputC_somtSetSymbolCopyName;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtSetSymbolCopyName)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtSetSymbolCopyValue)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtSetSymbolCopyValue,system)
		typedef somTP_SOMTTemplateOutputC_somtSetSymbolCopyValue *somTD_SOMTTemplateOutputC_somtSetSymbolCopyValue;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtSetSymbolCopyValue)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtSetSymbolCopyBoth)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtSetSymbolCopyBoth,system)
		typedef somTP_SOMTTemplateOutputC_somtSetSymbolCopyBoth *somTD_SOMTTemplateOutputC_somtSetSymbolCopyBoth;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtSetSymbolCopyBoth)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTTemplateOutputC_somtCheckSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtCheckSymbol,system)
		typedef somTP_SOMTTemplateOutputC_somtCheckSymbol *somTD_SOMTTemplateOutputC_somtCheckSymbol;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTTemplateOutputC_somtCheckSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string name);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtSetOutputFile)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* inout */ FILE *fp);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtSetOutputFile,system)
		typedef somTP_SOMTTemplateOutputC_somtSetOutputFile *somTD_SOMTTemplateOutputC_somtSetOutputFile;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtSetOutputFile)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* inout */ FILE *fp);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somto)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string tmplt);
		#pragma linkage(somTP_SOMTTemplateOutputC_somto,system)
		typedef somTP_SOMTTemplateOutputC_somto *somTD_SOMTTemplateOutputC_somto;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somto)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string tmplt);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtOutputComment)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string comment);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtOutputComment,system)
		typedef somTP_SOMTTemplateOutputC_somtOutputComment *somTD_SOMTTemplateOutputC_somtOutputComment;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtOutputComment)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string comment);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtOutputSection)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string sectionName);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtOutputSection,system)
		typedef somTP_SOMTTemplateOutputC_somtOutputSection *somTD_SOMTTemplateOutputC_somtOutputSection;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtOutputSection)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string sectionName);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtAddSectionDefinitions)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string defString);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtAddSectionDefinitions,system)
		typedef somTP_SOMTTemplateOutputC_somtAddSectionDefinitions *somTD_SOMTTemplateOutputC_somtAddSectionDefinitions;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtAddSectionDefinitions)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string defString);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMTTemplateOutputC_somtReadSectionDefinitions)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* inout */ FILE *fp);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtReadSectionDefinitions,system)
		typedef somTP_SOMTTemplateOutputC_somtReadSectionDefinitions *somTD_SOMTTemplateOutputC_somtReadSectionDefinitions;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMTTemplateOutputC_somtReadSectionDefinitions)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* inout */ FILE *fp);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTTemplateOutputC_somtExpandSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string s,
			/* in */ string buf);
		#pragma linkage(somTP_SOMTTemplateOutputC_somtExpandSymbol,system)
		typedef somTP_SOMTTemplateOutputC_somtExpandSymbol *somTD_SOMTTemplateOutputC_somtExpandSymbol;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTTemplateOutputC_somtExpandSymbol)(
			SOMTTemplateOutputC SOMSTAR somSelf,
			/* in */ string s,
			/* in */ string buf);
	#endif /* __IBMC__ */
	#ifndef SOMTTemplateOutputC_MajorVersion
		#define SOMTTemplateOutputC_MajorVersion   2
	#endif /* SOMTTemplateOutputC_MajorVersion */
	#ifndef SOMTTemplateOutputC_MinorVersion
		#define SOMTTemplateOutputC_MinorVersion   1
	#endif /* SOMTTemplateOutputC_MinorVersion */
	typedef struct SOMTTemplateOutputCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtCommentStyle;
		somMToken _set_somtCommentStyle;
		somMToken _get_somtLineLength;
		somMToken _set_somtLineLength;
		somMToken _set_somtCommentNewline;
		somMToken _get_somtCommentNewline;
		somMToken somtGetSymbol;
		somMToken somtSetSymbol;
		somMToken somtSetSymbolCopyName;
		somMToken somtSetSymbolCopyValue;
		somMToken somtSetSymbolCopyBoth;
		somMToken somtCheckSymbol;
		somMToken somtSetOutputFile;
		somMToken somto;
		somMToken somtOutputComment;
		somMToken somtOutputSection;
		somMToken somtAddSectionDefinitions;
		somMToken somtReadSectionDefinitions;
		somMToken somtExpandSymbol;
	} SOMTTemplateOutputCClassDataStructure;
	typedef struct SOMTTemplateOutputCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTTemplateOutputCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_sctmplt_Source) || defined(SOMTTemplateOutputC_Class_Source)
			SOMEXTERN struct SOMTTemplateOutputCClassDataStructure _SOMTTemplateOutputCClassData;
			#ifndef SOMTTemplateOutputCClassData
				#define SOMTTemplateOutputCClassData    _SOMTTemplateOutputCClassData
			#endif /* SOMTTemplateOutputCClassData */
		#else
			SOMEXTERN struct SOMTTemplateOutputCClassDataStructure * SOMLINK resolve_SOMTTemplateOutputCClassData(void);
			#ifndef SOMTTemplateOutputCClassData
				#define SOMTTemplateOutputCClassData    (*(resolve_SOMTTemplateOutputCClassData()))
			#endif /* SOMTTemplateOutputCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_sctmplt_Source) || defined(SOMTTemplateOutputC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_sctmplt_Source || SOMTTemplateOutputC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_sctmplt_Source || SOMTTemplateOutputC_Class_Source */
		struct SOMTTemplateOutputCClassDataStructure SOMDLINK SOMTTemplateOutputCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_sctmplt_Source) || defined(SOMTTemplateOutputC_Class_Source)
			SOMEXTERN struct SOMTTemplateOutputCCClassDataStructure _SOMTTemplateOutputCCClassData;
			#ifndef SOMTTemplateOutputCCClassData
				#define SOMTTemplateOutputCCClassData    _SOMTTemplateOutputCCClassData
			#endif /* SOMTTemplateOutputCCClassData */
		#else
			SOMEXTERN struct SOMTTemplateOutputCCClassDataStructure * SOMLINK resolve_SOMTTemplateOutputCCClassData(void);
			#ifndef SOMTTemplateOutputCCClassData
				#define SOMTTemplateOutputCCClassData    (*(resolve_SOMTTemplateOutputCCClassData()))
			#endif /* SOMTTemplateOutputCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_sctmplt_Source) || defined(SOMTTemplateOutputC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_sctmplt_Source || SOMTTemplateOutputC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_sctmplt_Source || SOMTTemplateOutputC_Class_Source */
		struct SOMTTemplateOutputCCClassDataStructure SOMDLINK SOMTTemplateOutputCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_sctmplt_Source) || defined(SOMTTemplateOutputC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_sctmplt_Source || SOMTTemplateOutputC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_sctmplt_Source || SOMTTemplateOutputC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTTemplateOutputCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTTemplateOutputC (SOMTTemplateOutputCClassData.classObject)
	#ifndef SOMGD_SOMTTemplateOutputC
		#if (defined(_SOMTTemplateOutputC) || defined(__SOMTTemplateOutputC))
			#undef _SOMTTemplateOutputC
			#undef __SOMTTemplateOutputC
			#define SOMGD_SOMTTemplateOutputC 1
		#else
			#define _SOMTTemplateOutputC _SOMCLASS_SOMTTemplateOutputC
		#endif /* _SOMTTemplateOutputC */
	#endif /* SOMGD_SOMTTemplateOutputC */
	#define SOMTTemplateOutputC_classObj _SOMCLASS_SOMTTemplateOutputC
	#define _SOMMTOKEN_SOMTTemplateOutputC(method) ((somMToken)(SOMTTemplateOutputCClassData.method))
	#ifndef SOMTTemplateOutputCNew
		#define SOMTTemplateOutputCNew() ( _SOMTTemplateOutputC ? \
			(SOMClass_somNew(_SOMTTemplateOutputC)) : \
			( SOMTTemplateOutputCNewClass( \
				SOMTTemplateOutputC_MajorVersion, \
				SOMTTemplateOutputC_MinorVersion), \
			SOMClass_somNew(_SOMTTemplateOutputC))) 
	#endif /* NewSOMTTemplateOutputC */
	#ifndef SOMTTemplateOutputC__set_somtCommentStyle
		#define SOMTTemplateOutputC__set_somtCommentStyle(somSelf,somtCommentStyle) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,_set_somtCommentStyle)  \
				(somSelf,somtCommentStyle)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somtCommentStyle
				#if defined(__set_somtCommentStyle)
					#undef __set_somtCommentStyle
					#define SOMGD___set_somtCommentStyle
				#else
					#define __set_somtCommentStyle SOMTTemplateOutputC__set_somtCommentStyle
				#endif
			#endif /* SOMGD___set_somtCommentStyle */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC__set_somtCommentStyle */
	#ifndef SOMTTemplateOutputC__get_somtCommentStyle
		#define SOMTTemplateOutputC__get_somtCommentStyle(somSelf) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,_get_somtCommentStyle)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtCommentStyle
				#if defined(__get_somtCommentStyle)
					#undef __get_somtCommentStyle
					#define SOMGD___get_somtCommentStyle
				#else
					#define __get_somtCommentStyle SOMTTemplateOutputC__get_somtCommentStyle
				#endif
			#endif /* SOMGD___get_somtCommentStyle */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC__get_somtCommentStyle */
	#ifndef SOMTTemplateOutputC__set_somtLineLength
		#define SOMTTemplateOutputC__set_somtLineLength(somSelf,somtLineLength) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,_set_somtLineLength)  \
				(somSelf,somtLineLength)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somtLineLength
				#if defined(__set_somtLineLength)
					#undef __set_somtLineLength
					#define SOMGD___set_somtLineLength
				#else
					#define __set_somtLineLength SOMTTemplateOutputC__set_somtLineLength
				#endif
			#endif /* SOMGD___set_somtLineLength */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC__set_somtLineLength */
	#ifndef SOMTTemplateOutputC__get_somtLineLength
		#define SOMTTemplateOutputC__get_somtLineLength(somSelf) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,_get_somtLineLength)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtLineLength
				#if defined(__get_somtLineLength)
					#undef __get_somtLineLength
					#define SOMGD___get_somtLineLength
				#else
					#define __get_somtLineLength SOMTTemplateOutputC__get_somtLineLength
				#endif
			#endif /* SOMGD___get_somtLineLength */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC__get_somtLineLength */
	#ifndef SOMTTemplateOutputC__set_somtCommentNewline
		#define SOMTTemplateOutputC__set_somtCommentNewline(somSelf,somtCommentNewline) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,_set_somtCommentNewline)  \
				(somSelf,somtCommentNewline)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somtCommentNewline
				#if defined(__set_somtCommentNewline)
					#undef __set_somtCommentNewline
					#define SOMGD___set_somtCommentNewline
				#else
					#define __set_somtCommentNewline SOMTTemplateOutputC__set_somtCommentNewline
				#endif
			#endif /* SOMGD___set_somtCommentNewline */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC__set_somtCommentNewline */
	#ifndef SOMTTemplateOutputC__get_somtCommentNewline
		#define SOMTTemplateOutputC__get_somtCommentNewline(somSelf) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,_get_somtCommentNewline)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtCommentNewline
				#if defined(__get_somtCommentNewline)
					#undef __get_somtCommentNewline
					#define SOMGD___get_somtCommentNewline
				#else
					#define __get_somtCommentNewline SOMTTemplateOutputC__get_somtCommentNewline
				#endif
			#endif /* SOMGD___get_somtCommentNewline */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC__get_somtCommentNewline */
	#ifndef SOMTTemplateOutputC_somtGetSymbol
		#define SOMTTemplateOutputC_somtGetSymbol(somSelf,name) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtGetSymbol)  \
				(somSelf,name)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetSymbol
				#if defined(_somtGetSymbol)
					#undef _somtGetSymbol
					#define SOMGD__somtGetSymbol
				#else
					#define _somtGetSymbol SOMTTemplateOutputC_somtGetSymbol
				#endif
			#endif /* SOMGD__somtGetSymbol */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtGetSymbol */
	#ifndef SOMTTemplateOutputC_somtSetSymbol
		#define SOMTTemplateOutputC_somtSetSymbol(somSelf,name,value) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtSetSymbol)  \
				(somSelf,name,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtSetSymbol
				#if defined(_somtSetSymbol)
					#undef _somtSetSymbol
					#define SOMGD__somtSetSymbol
				#else
					#define _somtSetSymbol SOMTTemplateOutputC_somtSetSymbol
				#endif
			#endif /* SOMGD__somtSetSymbol */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtSetSymbol */
	#ifndef SOMTTemplateOutputC_somtSetSymbolCopyName
		#define SOMTTemplateOutputC_somtSetSymbolCopyName(somSelf,name,value) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtSetSymbolCopyName)  \
				(somSelf,name,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtSetSymbolCopyName
				#if defined(_somtSetSymbolCopyName)
					#undef _somtSetSymbolCopyName
					#define SOMGD__somtSetSymbolCopyName
				#else
					#define _somtSetSymbolCopyName SOMTTemplateOutputC_somtSetSymbolCopyName
				#endif
			#endif /* SOMGD__somtSetSymbolCopyName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtSetSymbolCopyName */
	#ifndef SOMTTemplateOutputC_somtSetSymbolCopyValue
		#define SOMTTemplateOutputC_somtSetSymbolCopyValue(somSelf,name,value) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtSetSymbolCopyValue)  \
				(somSelf,name,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtSetSymbolCopyValue
				#if defined(_somtSetSymbolCopyValue)
					#undef _somtSetSymbolCopyValue
					#define SOMGD__somtSetSymbolCopyValue
				#else
					#define _somtSetSymbolCopyValue SOMTTemplateOutputC_somtSetSymbolCopyValue
				#endif
			#endif /* SOMGD__somtSetSymbolCopyValue */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtSetSymbolCopyValue */
	#ifndef SOMTTemplateOutputC_somtSetSymbolCopyBoth
		#define SOMTTemplateOutputC_somtSetSymbolCopyBoth(somSelf,name,value) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtSetSymbolCopyBoth)  \
				(somSelf,name,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtSetSymbolCopyBoth
				#if defined(_somtSetSymbolCopyBoth)
					#undef _somtSetSymbolCopyBoth
					#define SOMGD__somtSetSymbolCopyBoth
				#else
					#define _somtSetSymbolCopyBoth SOMTTemplateOutputC_somtSetSymbolCopyBoth
				#endif
			#endif /* SOMGD__somtSetSymbolCopyBoth */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtSetSymbolCopyBoth */
	#ifndef SOMTTemplateOutputC_somtCheckSymbol
		#define SOMTTemplateOutputC_somtCheckSymbol(somSelf,name) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtCheckSymbol)  \
				(somSelf,name)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtCheckSymbol
				#if defined(_somtCheckSymbol)
					#undef _somtCheckSymbol
					#define SOMGD__somtCheckSymbol
				#else
					#define _somtCheckSymbol SOMTTemplateOutputC_somtCheckSymbol
				#endif
			#endif /* SOMGD__somtCheckSymbol */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtCheckSymbol */
	#ifndef SOMTTemplateOutputC_somtSetOutputFile
		#define SOMTTemplateOutputC_somtSetOutputFile(somSelf,fp) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtSetOutputFile)  \
				(somSelf,fp)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtSetOutputFile
				#if defined(_somtSetOutputFile)
					#undef _somtSetOutputFile
					#define SOMGD__somtSetOutputFile
				#else
					#define _somtSetOutputFile SOMTTemplateOutputC_somtSetOutputFile
				#endif
			#endif /* SOMGD__somtSetOutputFile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtSetOutputFile */
	#ifndef SOMTTemplateOutputC_somto
		#define SOMTTemplateOutputC_somto(somSelf,tmplt) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somto)  \
				(somSelf,tmplt)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somto
				#if defined(_somto)
					#undef _somto
					#define SOMGD__somto
				#else
					#define _somto SOMTTemplateOutputC_somto
				#endif
			#endif /* SOMGD__somto */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somto */
	#ifndef SOMTTemplateOutputC_somtOutputComment
		#define SOMTTemplateOutputC_somtOutputComment(somSelf,comment) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtOutputComment)  \
				(somSelf,comment)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtOutputComment
				#if defined(_somtOutputComment)
					#undef _somtOutputComment
					#define SOMGD__somtOutputComment
				#else
					#define _somtOutputComment SOMTTemplateOutputC_somtOutputComment
				#endif
			#endif /* SOMGD__somtOutputComment */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtOutputComment */
	#ifndef SOMTTemplateOutputC_somtOutputSection
		#define SOMTTemplateOutputC_somtOutputSection(somSelf,sectionName) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtOutputSection)  \
				(somSelf,sectionName)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtOutputSection
				#if defined(_somtOutputSection)
					#undef _somtOutputSection
					#define SOMGD__somtOutputSection
				#else
					#define _somtOutputSection SOMTTemplateOutputC_somtOutputSection
				#endif
			#endif /* SOMGD__somtOutputSection */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtOutputSection */
	#ifndef SOMTTemplateOutputC_somtAddSectionDefinitions
		#define SOMTTemplateOutputC_somtAddSectionDefinitions(somSelf,defString) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtAddSectionDefinitions)  \
				(somSelf,defString)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtAddSectionDefinitions
				#if defined(_somtAddSectionDefinitions)
					#undef _somtAddSectionDefinitions
					#define SOMGD__somtAddSectionDefinitions
				#else
					#define _somtAddSectionDefinitions SOMTTemplateOutputC_somtAddSectionDefinitions
				#endif
			#endif /* SOMGD__somtAddSectionDefinitions */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtAddSectionDefinitions */
	#ifndef SOMTTemplateOutputC_somtReadSectionDefinitions
		#define SOMTTemplateOutputC_somtReadSectionDefinitions(somSelf,fp) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtReadSectionDefinitions)  \
				(somSelf,fp)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtReadSectionDefinitions
				#if defined(_somtReadSectionDefinitions)
					#undef _somtReadSectionDefinitions
					#define SOMGD__somtReadSectionDefinitions
				#else
					#define _somtReadSectionDefinitions SOMTTemplateOutputC_somtReadSectionDefinitions
				#endif
			#endif /* SOMGD__somtReadSectionDefinitions */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtReadSectionDefinitions */
	#ifndef SOMTTemplateOutputC_somtExpandSymbol
		#define SOMTTemplateOutputC_somtExpandSymbol(somSelf,s,buf) \
			SOM_Resolve(somSelf,SOMTTemplateOutputC,somtExpandSymbol)  \
				(somSelf,s,buf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtExpandSymbol
				#if defined(_somtExpandSymbol)
					#undef _somtExpandSymbol
					#define SOMGD__somtExpandSymbol
				#else
					#define _somtExpandSymbol SOMTTemplateOutputC_somtExpandSymbol
				#endif
			#endif /* SOMGD__somtExpandSymbol */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTemplateOutputC_somtExpandSymbol */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTTemplateOutputC_somInit SOMObject_somInit
#define SOMTTemplateOutputC_somUninit SOMObject_somUninit
#define SOMTTemplateOutputC_somFree SOMObject_somFree
#define SOMTTemplateOutputC_somGetClass SOMObject_somGetClass
#define SOMTTemplateOutputC_somGetClassName SOMObject_somGetClassName
#define SOMTTemplateOutputC_somGetSize SOMObject_somGetSize
#define SOMTTemplateOutputC_somIsA SOMObject_somIsA
#define SOMTTemplateOutputC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTTemplateOutputC_somRespondsTo SOMObject_somRespondsTo
#define SOMTTemplateOutputC_somDispatch SOMObject_somDispatch
#define SOMTTemplateOutputC_somClassDispatch SOMObject_somClassDispatch
#define SOMTTemplateOutputC_somCastObj SOMObject_somCastObj
#define SOMTTemplateOutputC_somResetObj SOMObject_somResetObj
#define SOMTTemplateOutputC_somPrintSelf SOMObject_somPrintSelf
#define SOMTTemplateOutputC_somDumpSelf SOMObject_somDumpSelf
#define SOMTTemplateOutputC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTTemplateOutputC_somDefaultInit SOMObject_somDefaultInit
#define SOMTTemplateOutputC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTTemplateOutputC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTTemplateOutputC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTTemplateOutputC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTTemplateOutputC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTTemplateOutputC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTTemplateOutputC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTTemplateOutputC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTTemplateOutputC_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_sctmplt_Header_h */
