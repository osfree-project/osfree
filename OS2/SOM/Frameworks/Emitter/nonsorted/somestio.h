/* generated from somestio.idl */
/* internal conditional is SOM_Module_somestio_Source */
#ifndef SOM_Module_somestio_Header_h
	#define SOM_Module_somestio_Header_h 1
	#include <som.h>
	#include <somos.h>
	#include <somobj.h>
	#include <somdserv.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#include <xnaming.h>
	#include <naming.h>
	#include <somdcprx.h>
	#include <somproxy.h>
	#include <somdobj.h>
	#include <unotypes.h>
	#include <xmscssae.h>
	#include <omgidobj.h>
	#include <omgestio.h>
		#ifndef _IDL_somStream_StreamIO_defined
			#define _IDL_somStream_StreamIO_defined
			typedef SOMObject somStream_StreamIO;
		#endif /* _IDL_somStream_StreamIO_defined */
		#ifndef _IDL_SEQUENCE_somStream_StreamIO_defined
			#define _IDL_SEQUENCE_somStream_StreamIO_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somStream_StreamIO SOMSTAR ,sequence(somStream_StreamIO));
		#endif /* _IDL_SEQUENCE_somStream_StreamIO_defined */
		#ifndef _IDL_somStream_MemoryStreamIO_defined
			#define _IDL_somStream_MemoryStreamIO_defined
			typedef SOMObject somStream_MemoryStreamIO;
		#endif /* _IDL_somStream_MemoryStreamIO_defined */
		#ifndef _IDL_SEQUENCE_somStream_MemoryStreamIO_defined
			#define _IDL_SEQUENCE_somStream_MemoryStreamIO_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somStream_MemoryStreamIO SOMSTAR ,sequence(somStream_MemoryStreamIO));
		#endif /* _IDL_SEQUENCE_somStream_MemoryStreamIO_defined */
		#ifndef _IDL_somStream_Streamable_defined
			#define _IDL_somStream_Streamable_defined
			typedef SOMObject somStream_Streamable;
		#endif /* _IDL_somStream_Streamable_defined */
		#ifndef _IDL_SEQUENCE_somStream_Streamable_defined
			#define _IDL_SEQUENCE_somStream_Streamable_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somStream_Streamable SOMSTAR ,sequence(somStream_Streamable));
		#endif /* _IDL_SEQUENCE_somStream_Streamable_defined */
		#ifndef _IDL_SEQUENCE_octet_defined
			#define _IDL_SEQUENCE_octet_defined
			SOM_SEQUENCE_TYPEDEF(octet);
		#endif /* _IDL_SEQUENCE_octet_defined */
		typedef _IDL_SEQUENCE_octet somStream_seq_octet;
		#ifndef _IDL_SEQUENCE_somStream_seq_octet_defined
			#define _IDL_SEQUENCE_somStream_seq_octet_defined
			SOM_SEQUENCE_TYPEDEF(somStream_seq_octet);
		#endif /* _IDL_SEQUENCE_somStream_seq_octet_defined */
#ifndef somStream_GENERAL
	#define somStream_GENERAL   54000L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_GENERAL
			#if defined(GENERAL)
				#undef GENERAL
				#define SOMGD_GENERAL
			#else
				#define GENERAL somStream_GENERAL
			#endif
		#endif /* SOMGD_GENERAL */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_GENERAL */
#ifndef somStream_SEMAPHORE_CREATE
	#define somStream_SEMAPHORE_CREATE   54001L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SEMAPHORE_CREATE
			#if defined(SEMAPHORE_CREATE)
				#undef SEMAPHORE_CREATE
				#define SOMGD_SEMAPHORE_CREATE
			#else
				#define SEMAPHORE_CREATE somStream_SEMAPHORE_CREATE
			#endif
		#endif /* SOMGD_SEMAPHORE_CREATE */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_SEMAPHORE_CREATE */
#ifndef somStream_SEMAPHORE_REQUEST
	#define somStream_SEMAPHORE_REQUEST   54002L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SEMAPHORE_REQUEST
			#if defined(SEMAPHORE_REQUEST)
				#undef SEMAPHORE_REQUEST
				#define SOMGD_SEMAPHORE_REQUEST
			#else
				#define SEMAPHORE_REQUEST somStream_SEMAPHORE_REQUEST
			#endif
		#endif /* SOMGD_SEMAPHORE_REQUEST */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_SEMAPHORE_REQUEST */
#ifndef somStream_SEMAPHORE_RELEASE
	#define somStream_SEMAPHORE_RELEASE   54003L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_SEMAPHORE_RELEASE
			#if defined(SEMAPHORE_RELEASE)
				#undef SEMAPHORE_RELEASE
				#define SOMGD_SEMAPHORE_RELEASE
			#else
				#define SEMAPHORE_RELEASE somStream_SEMAPHORE_RELEASE
			#endif
		#endif /* SOMGD_SEMAPHORE_RELEASE */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_SEMAPHORE_RELEASE */
#ifndef somStream_FACTORY_FINDER_LOAD
	#define somStream_FACTORY_FINDER_LOAD   54004L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FACTORY_FINDER_LOAD
			#if defined(FACTORY_FINDER_LOAD)
				#undef FACTORY_FINDER_LOAD
				#define SOMGD_FACTORY_FINDER_LOAD
			#else
				#define FACTORY_FINDER_LOAD somStream_FACTORY_FINDER_LOAD
			#endif
		#endif /* SOMGD_FACTORY_FINDER_LOAD */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_FACTORY_FINDER_LOAD */
#ifndef somStream_FACTORY_FINDER_NEW
	#define somStream_FACTORY_FINDER_NEW   54005L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FACTORY_FINDER_NEW
			#if defined(FACTORY_FINDER_NEW)
				#undef FACTORY_FINDER_NEW
				#define SOMGD_FACTORY_FINDER_NEW
			#else
				#define FACTORY_FINDER_NEW somStream_FACTORY_FINDER_NEW
			#endif
		#endif /* SOMGD_FACTORY_FINDER_NEW */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_FACTORY_FINDER_NEW */
#ifndef somStream_FACTORY_FINDER_CLASS
	#define somStream_FACTORY_FINDER_CLASS   54006L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FACTORY_FINDER_CLASS
			#if defined(FACTORY_FINDER_CLASS)
				#undef FACTORY_FINDER_CLASS
				#define SOMGD_FACTORY_FINDER_CLASS
			#else
				#define FACTORY_FINDER_CLASS somStream_FACTORY_FINDER_CLASS
			#endif
		#endif /* SOMGD_FACTORY_FINDER_CLASS */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_FACTORY_FINDER_CLASS */
#ifndef somStream_FACTORY_FINDER_METHOD
	#define somStream_FACTORY_FINDER_METHOD   54007L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FACTORY_FINDER_METHOD
			#if defined(FACTORY_FINDER_METHOD)
				#undef FACTORY_FINDER_METHOD
				#define SOMGD_FACTORY_FINDER_METHOD
			#else
				#define FACTORY_FINDER_METHOD somStream_FACTORY_FINDER_METHOD
			#endif
		#endif /* SOMGD_FACTORY_FINDER_METHOD */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_FACTORY_FINDER_METHOD */
#ifndef somStream_FIND_FACTORY
	#define somStream_FIND_FACTORY   54008L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FIND_FACTORY
			#if defined(FIND_FACTORY)
				#undef FIND_FACTORY
				#define SOMGD_FIND_FACTORY
			#else
				#define FIND_FACTORY somStream_FIND_FACTORY
			#endif
		#endif /* SOMGD_FIND_FACTORY */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_FIND_FACTORY */
#ifndef somStream_NEW_NOT_STREAMABLE
	#define somStream_NEW_NOT_STREAMABLE   54009L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_NEW_NOT_STREAMABLE
			#if defined(NEW_NOT_STREAMABLE)
				#undef NEW_NOT_STREAMABLE
				#define SOMGD_NEW_NOT_STREAMABLE
			#else
				#define NEW_NOT_STREAMABLE somStream_NEW_NOT_STREAMABLE
			#endif
		#endif /* SOMGD_NEW_NOT_STREAMABLE */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_NEW_NOT_STREAMABLE */
#ifndef somStream_OBJ_NOT_STREAMABLE
	#define somStream_OBJ_NOT_STREAMABLE   54010L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_OBJ_NOT_STREAMABLE
			#if defined(OBJ_NOT_STREAMABLE)
				#undef OBJ_NOT_STREAMABLE
				#define SOMGD_OBJ_NOT_STREAMABLE
			#else
				#define OBJ_NOT_STREAMABLE somStream_OBJ_NOT_STREAMABLE
			#endif
		#endif /* SOMGD_OBJ_NOT_STREAMABLE */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_OBJ_NOT_STREAMABLE */
#ifndef somStream_FACTORY_FINDER
	#define somStream_FACTORY_FINDER   54011L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FACTORY_FINDER
			#if defined(FACTORY_FINDER)
				#undef FACTORY_FINDER
				#define SOMGD_FACTORY_FINDER
			#else
				#define FACTORY_FINDER somStream_FACTORY_FINDER
			#endif
		#endif /* SOMGD_FACTORY_FINDER */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_FACTORY_FINDER */
#ifndef somStream_METHOD_IS_ABSTRACT
	#define somStream_METHOD_IS_ABSTRACT   54012L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_METHOD_IS_ABSTRACT
			#if defined(METHOD_IS_ABSTRACT)
				#undef METHOD_IS_ABSTRACT
				#define SOMGD_METHOD_IS_ABSTRACT
			#else
				#define METHOD_IS_ABSTRACT somStream_METHOD_IS_ABSTRACT
			#endif
		#endif /* SOMGD_METHOD_IS_ABSTRACT */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_METHOD_IS_ABSTRACT */
#ifndef somStream_OBJ_REPEAT_REF_MISMATCH
	#define somStream_OBJ_REPEAT_REF_MISMATCH   54013L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_OBJ_REPEAT_REF_MISMATCH
			#if defined(OBJ_REPEAT_REF_MISMATCH)
				#undef OBJ_REPEAT_REF_MISMATCH
				#define SOMGD_OBJ_REPEAT_REF_MISMATCH
			#else
				#define OBJ_REPEAT_REF_MISMATCH somStream_OBJ_REPEAT_REF_MISMATCH
			#endif
		#endif /* SOMGD_OBJ_REPEAT_REF_MISMATCH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_OBJ_REPEAT_REF_MISMATCH */
#ifndef somStream_UNABLE_TO_CREATE_STREAMIO
	#define somStream_UNABLE_TO_CREATE_STREAMIO   54014L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_CREATE_STREAMIO
			#if defined(UNABLE_TO_CREATE_STREAMIO)
				#undef UNABLE_TO_CREATE_STREAMIO
				#define SOMGD_UNABLE_TO_CREATE_STREAMIO
			#else
				#define UNABLE_TO_CREATE_STREAMIO somStream_UNABLE_TO_CREATE_STREAMIO
			#endif
		#endif /* SOMGD_UNABLE_TO_CREATE_STREAMIO */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_CREATE_STREAMIO */
#ifndef somStream_STREAM_CREATE_WITH_TYPES
	#define somStream_STREAM_CREATE_WITH_TYPES   54015L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_STREAM_CREATE_WITH_TYPES
			#if defined(STREAM_CREATE_WITH_TYPES)
				#undef STREAM_CREATE_WITH_TYPES
				#define SOMGD_STREAM_CREATE_WITH_TYPES
			#else
				#define STREAM_CREATE_WITH_TYPES somStream_STREAM_CREATE_WITH_TYPES
			#endif
		#endif /* SOMGD_STREAM_CREATE_WITH_TYPES */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_STREAM_CREATE_WITH_TYPES */
#ifndef somStream_UNABLE_TO_CREATE_STREAM
	#define somStream_UNABLE_TO_CREATE_STREAM   54016L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_CREATE_STREAM
			#if defined(UNABLE_TO_CREATE_STREAM)
				#undef UNABLE_TO_CREATE_STREAM
				#define SOMGD_UNABLE_TO_CREATE_STREAM
			#else
				#define UNABLE_TO_CREATE_STREAM somStream_UNABLE_TO_CREATE_STREAM
			#endif
		#endif /* SOMGD_UNABLE_TO_CREATE_STREAM */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_CREATE_STREAM */
#ifndef somStream_INVALID_KEY_LENGTH
	#define somStream_INVALID_KEY_LENGTH   54017L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_INVALID_KEY_LENGTH
			#if defined(INVALID_KEY_LENGTH)
				#undef INVALID_KEY_LENGTH
				#define SOMGD_INVALID_KEY_LENGTH
			#else
				#define INVALID_KEY_LENGTH somStream_INVALID_KEY_LENGTH
			#endif
		#endif /* SOMGD_INVALID_KEY_LENGTH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_INVALID_KEY_LENGTH */
#ifndef somStream_ALREADY_STREAMED_PARMS
	#define somStream_ALREADY_STREAMED_PARMS   54018L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ALREADY_STREAMED_PARMS
			#if defined(ALREADY_STREAMED_PARMS)
				#undef ALREADY_STREAMED_PARMS
				#define SOMGD_ALREADY_STREAMED_PARMS
			#else
				#define ALREADY_STREAMED_PARMS somStream_ALREADY_STREAMED_PARMS
			#endif
		#endif /* SOMGD_ALREADY_STREAMED_PARMS */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_ALREADY_STREAMED_PARMS */
#ifndef somStream_BAD_BUFFER_PARAMETER
	#define somStream_BAD_BUFFER_PARAMETER   54019L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_BAD_BUFFER_PARAMETER
			#if defined(BAD_BUFFER_PARAMETER)
				#undef BAD_BUFFER_PARAMETER
				#define SOMGD_BAD_BUFFER_PARAMETER
			#else
				#define BAD_BUFFER_PARAMETER somStream_BAD_BUFFER_PARAMETER
			#endif
		#endif /* SOMGD_BAD_BUFFER_PARAMETER */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_BAD_BUFFER_PARAMETER */
#ifndef somStream_ICONV_FAILURE
	#define somStream_ICONV_FAILURE   54020L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ICONV_FAILURE
			#if defined(ICONV_FAILURE)
				#undef ICONV_FAILURE
				#define SOMGD_ICONV_FAILURE
			#else
				#define ICONV_FAILURE somStream_ICONV_FAILURE
			#endif
		#endif /* SOMGD_ICONV_FAILURE */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_ICONV_FAILURE */
#ifndef somStream_READ_PASSED_END_OF_STREAM
	#define somStream_READ_PASSED_END_OF_STREAM   54021L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_READ_PASSED_END_OF_STREAM
			#if defined(READ_PASSED_END_OF_STREAM)
				#undef READ_PASSED_END_OF_STREAM
				#define SOMGD_READ_PASSED_END_OF_STREAM
			#else
				#define READ_PASSED_END_OF_STREAM somStream_READ_PASSED_END_OF_STREAM
			#endif
		#endif /* SOMGD_READ_PASSED_END_OF_STREAM */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_READ_PASSED_END_OF_STREAM */
#ifndef somStream_REPEAT_REF_NUMBER_NOT_FOUND
	#define somStream_REPEAT_REF_NUMBER_NOT_FOUND   54022L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_REPEAT_REF_NUMBER_NOT_FOUND
			#if defined(REPEAT_REF_NUMBER_NOT_FOUND)
				#undef REPEAT_REF_NUMBER_NOT_FOUND
				#define SOMGD_REPEAT_REF_NUMBER_NOT_FOUND
			#else
				#define REPEAT_REF_NUMBER_NOT_FOUND somStream_REPEAT_REF_NUMBER_NOT_FOUND
			#endif
		#endif /* SOMGD_REPEAT_REF_NUMBER_NOT_FOUND */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_REPEAT_REF_NUMBER_NOT_FOUND */
#ifndef somStream_OBJ_NIL_DATA_MISMATCH
	#define somStream_OBJ_NIL_DATA_MISMATCH   54023L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_OBJ_NIL_DATA_MISMATCH
			#if defined(OBJ_NIL_DATA_MISMATCH)
				#undef OBJ_NIL_DATA_MISMATCH
				#define SOMGD_OBJ_NIL_DATA_MISMATCH
			#else
				#define OBJ_NIL_DATA_MISMATCH somStream_OBJ_NIL_DATA_MISMATCH
			#endif
		#endif /* SOMGD_OBJ_NIL_DATA_MISMATCH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_OBJ_NIL_DATA_MISMATCH */
#ifndef somStream_OBJ_STRINGIFIED_REF_MISMATCH
	#define somStream_OBJ_STRINGIFIED_REF_MISMATCH   54024L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_OBJ_STRINGIFIED_REF_MISMATCH
			#if defined(OBJ_STRINGIFIED_REF_MISMATCH)
				#undef OBJ_STRINGIFIED_REF_MISMATCH
				#define SOMGD_OBJ_STRINGIFIED_REF_MISMATCH
			#else
				#define OBJ_STRINGIFIED_REF_MISMATCH somStream_OBJ_STRINGIFIED_REF_MISMATCH
			#endif
		#endif /* SOMGD_OBJ_STRINGIFIED_REF_MISMATCH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_OBJ_STRINGIFIED_REF_MISMATCH */
#ifndef somStream_FOUND_UNKNOWN_TAG
	#define somStream_FOUND_UNKNOWN_TAG   54025L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FOUND_UNKNOWN_TAG
			#if defined(FOUND_UNKNOWN_TAG)
				#undef FOUND_UNKNOWN_TAG
				#define SOMGD_FOUND_UNKNOWN_TAG
			#else
				#define FOUND_UNKNOWN_TAG somStream_FOUND_UNKNOWN_TAG
			#endif
		#endif /* SOMGD_FOUND_UNKNOWN_TAG */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_FOUND_UNKNOWN_TAG */
#ifndef somStream_UNABLE_TO_READ_SHORT
	#define somStream_UNABLE_TO_READ_SHORT   54026L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_SHORT
			#if defined(UNABLE_TO_READ_SHORT)
				#undef UNABLE_TO_READ_SHORT
				#define SOMGD_UNABLE_TO_READ_SHORT
			#else
				#define UNABLE_TO_READ_SHORT somStream_UNABLE_TO_READ_SHORT
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_SHORT */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_SHORT */
#ifndef somStream_UNABLE_TO_READ_LONG
	#define somStream_UNABLE_TO_READ_LONG   54027L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_LONG
			#if defined(UNABLE_TO_READ_LONG)
				#undef UNABLE_TO_READ_LONG
				#define SOMGD_UNABLE_TO_READ_LONG
			#else
				#define UNABLE_TO_READ_LONG somStream_UNABLE_TO_READ_LONG
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_LONG */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_LONG */
#ifndef somStream_UNABLE_TO_READ_USHORT
	#define somStream_UNABLE_TO_READ_USHORT   54028L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_USHORT
			#if defined(UNABLE_TO_READ_USHORT)
				#undef UNABLE_TO_READ_USHORT
				#define SOMGD_UNABLE_TO_READ_USHORT
			#else
				#define UNABLE_TO_READ_USHORT somStream_UNABLE_TO_READ_USHORT
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_USHORT */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_USHORT */
#ifndef somStream_UNABLE_TO_READ_ULONG
	#define somStream_UNABLE_TO_READ_ULONG   54029L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_ULONG
			#if defined(UNABLE_TO_READ_ULONG)
				#undef UNABLE_TO_READ_ULONG
				#define SOMGD_UNABLE_TO_READ_ULONG
			#else
				#define UNABLE_TO_READ_ULONG somStream_UNABLE_TO_READ_ULONG
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_ULONG */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_ULONG */
#ifndef somStream_UNABLE_TO_READ_STRING
	#define somStream_UNABLE_TO_READ_STRING   54030L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_STRING
			#if defined(UNABLE_TO_READ_STRING)
				#undef UNABLE_TO_READ_STRING
				#define SOMGD_UNABLE_TO_READ_STRING
			#else
				#define UNABLE_TO_READ_STRING somStream_UNABLE_TO_READ_STRING
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_STRING */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_STRING */
#ifndef somStream_UNABLE_TO_READ_CHAR
	#define somStream_UNABLE_TO_READ_CHAR   54031L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_CHAR
			#if defined(UNABLE_TO_READ_CHAR)
				#undef UNABLE_TO_READ_CHAR
				#define SOMGD_UNABLE_TO_READ_CHAR
			#else
				#define UNABLE_TO_READ_CHAR somStream_UNABLE_TO_READ_CHAR
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_CHAR */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_CHAR */
#ifndef somStream_UNABLE_TO_READ_FLOAT
	#define somStream_UNABLE_TO_READ_FLOAT   54032L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_FLOAT
			#if defined(UNABLE_TO_READ_FLOAT)
				#undef UNABLE_TO_READ_FLOAT
				#define SOMGD_UNABLE_TO_READ_FLOAT
			#else
				#define UNABLE_TO_READ_FLOAT somStream_UNABLE_TO_READ_FLOAT
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_FLOAT */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_FLOAT */
#ifndef somStream_UNABLE_TO_READ_DOUBLE
	#define somStream_UNABLE_TO_READ_DOUBLE   54033L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_DOUBLE
			#if defined(UNABLE_TO_READ_DOUBLE)
				#undef UNABLE_TO_READ_DOUBLE
				#define SOMGD_UNABLE_TO_READ_DOUBLE
			#else
				#define UNABLE_TO_READ_DOUBLE somStream_UNABLE_TO_READ_DOUBLE
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_DOUBLE */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_DOUBLE */
#ifndef somStream_UNABLE_TO_READ_OCTET
	#define somStream_UNABLE_TO_READ_OCTET   54034L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_OCTET
			#if defined(UNABLE_TO_READ_OCTET)
				#undef UNABLE_TO_READ_OCTET
				#define SOMGD_UNABLE_TO_READ_OCTET
			#else
				#define UNABLE_TO_READ_OCTET somStream_UNABLE_TO_READ_OCTET
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_OCTET */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_OCTET */
#ifndef somStream_UNABLE_TO_READ_BOOLEAN
	#define somStream_UNABLE_TO_READ_BOOLEAN   54035L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_BOOLEAN
			#if defined(UNABLE_TO_READ_BOOLEAN)
				#undef UNABLE_TO_READ_BOOLEAN
				#define SOMGD_UNABLE_TO_READ_BOOLEAN
			#else
				#define UNABLE_TO_READ_BOOLEAN somStream_UNABLE_TO_READ_BOOLEAN
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_BOOLEAN */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_BOOLEAN */
#ifndef somStream_UNABLE_TO_READ_OBJECT
	#define somStream_UNABLE_TO_READ_OBJECT   54036L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_UNABLE_TO_READ_OBJECT
			#if defined(UNABLE_TO_READ_OBJECT)
				#undef UNABLE_TO_READ_OBJECT
				#define SOMGD_UNABLE_TO_READ_OBJECT
			#else
				#define UNABLE_TO_READ_OBJECT somStream_UNABLE_TO_READ_OBJECT
			#endif
		#endif /* SOMGD_UNABLE_TO_READ_OBJECT */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_UNABLE_TO_READ_OBJECT */
#ifndef somStream_READ_FROM_EMPTY_STREAM
	#define somStream_READ_FROM_EMPTY_STREAM   54037L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_READ_FROM_EMPTY_STREAM
			#if defined(READ_FROM_EMPTY_STREAM)
				#undef READ_FROM_EMPTY_STREAM
				#define SOMGD_READ_FROM_EMPTY_STREAM
			#else
				#define READ_FROM_EMPTY_STREAM somStream_READ_FROM_EMPTY_STREAM
			#endif
		#endif /* SOMGD_READ_FROM_EMPTY_STREAM */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* somStream_READ_FROM_EMPTY_STREAM */
		#ifndef somStream_StreamIO_object_tags_defined
			#define somStream_StreamIO_object_tags_defined
			typedef unsigned long somStream_StreamIO_object_tags;
			#ifndef somStream_StreamIO_KeyData
				#define somStream_StreamIO_KeyData 1UL
			#endif /* somStream_StreamIO_KeyData */
			#ifndef somStream_StreamIO_RepeatRef
				#define somStream_StreamIO_RepeatRef 2UL
			#endif /* somStream_StreamIO_RepeatRef */
			#ifndef somStream_StreamIO_NilData
				#define somStream_StreamIO_NilData 3UL
			#endif /* somStream_StreamIO_NilData */
			#ifndef somStream_StreamIO_StringifiedRef
				#define somStream_StreamIO_StringifiedRef 4UL
			#endif /* somStream_StreamIO_StringifiedRef */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_object_tags
					#if defined(object_tags)
						#undef object_tags
						#define SOMGD_object_tags
					#else
						#define object_tags somStream_StreamIO_object_tags
					#endif
				#endif /* SOMGD_object_tags */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somStream_StreamIO_object_tags_defined */
		#ifdef __IBMC__
			typedef boolean (somTP_somStream_StreamIO_already_streamed)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR obj,
				/* in */ SOMObject SOMSTAR class_obj);
			#pragma linkage(somTP_somStream_StreamIO_already_streamed,system)
			typedef somTP_somStream_StreamIO_already_streamed *somTD_somStream_StreamIO_already_streamed;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_somStream_StreamIO_already_streamed)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR obj,
				/* in */ SOMObject SOMSTAR class_obj);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somStream_StreamIO_reset)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somStream_StreamIO_reset,system)
			typedef somTP_somStream_StreamIO_reset *somTD_somStream_StreamIO_reset;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somStream_StreamIO_reset)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somStream_StreamIO_set_buffer)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ somStream_seq_octet *buffer);
			#pragma linkage(somTP_somStream_StreamIO_set_buffer,system)
			typedef somTP_somStream_StreamIO_set_buffer *somTD_somStream_StreamIO_set_buffer;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somStream_StreamIO_set_buffer)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ somStream_seq_octet *buffer);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef somStream_seq_octet (somTP_somStream_StreamIO_get_buffer)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somStream_StreamIO_get_buffer,system)
			typedef somTP_somStream_StreamIO_get_buffer *somTD_somStream_StreamIO_get_buffer;
		#else /* __IBMC__ */
			typedef somStream_seq_octet (SOMLINK * somTD_somStream_StreamIO_get_buffer)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somStream_StreamIO_clear_buffer)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somStream_StreamIO_clear_buffer,system)
			typedef somTP_somStream_StreamIO_clear_buffer *somTD_somStream_StreamIO_clear_buffer;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somStream_StreamIO_clear_buffer)(
				somStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef somStream_StreamIO_MajorVersion
			#define somStream_StreamIO_MajorVersion   3
		#endif /* somStream_StreamIO_MajorVersion */
		#ifndef somStream_StreamIO_MinorVersion
			#define somStream_StreamIO_MinorVersion   0
		#endif /* somStream_StreamIO_MinorVersion */
		typedef struct somStream_StreamIOClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken private0;
			somMToken private1;
			somMToken private2;
			somMToken private3;
			somMToken private4;
			somMToken reset;
			somMToken get_buffer;
			somMToken set_buffer;
			somMToken clear_buffer;
			somMToken private9;
			somMToken already_streamed;
			somMToken private11;
		} somStream_StreamIOClassDataStructure;
		typedef struct somStream_StreamIOCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somStream_StreamIOCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somestio_Source) || defined(somStream_StreamIO_Class_Source)
				SOMEXTERN struct somStream_StreamIOClassDataStructure _somStream_StreamIOClassData;
				#ifndef somStream_StreamIOClassData
					#define somStream_StreamIOClassData    _somStream_StreamIOClassData
				#endif /* somStream_StreamIOClassData */
			#else
				SOMEXTERN struct somStream_StreamIOClassDataStructure * SOMLINK resolve_somStream_StreamIOClassData(void);
				#ifndef somStream_StreamIOClassData
					#define somStream_StreamIOClassData    (*(resolve_somStream_StreamIOClassData()))
				#endif /* somStream_StreamIOClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somestio_Source) || defined(somStream_StreamIO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somestio_Source || somStream_StreamIO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somestio_Source || somStream_StreamIO_Class_Source */
			struct somStream_StreamIOClassDataStructure SOMDLINK somStream_StreamIOClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somestio_Source) || defined(somStream_StreamIO_Class_Source)
				SOMEXTERN struct somStream_StreamIOCClassDataStructure _somStream_StreamIOCClassData;
				#ifndef somStream_StreamIOCClassData
					#define somStream_StreamIOCClassData    _somStream_StreamIOCClassData
				#endif /* somStream_StreamIOCClassData */
			#else
				SOMEXTERN struct somStream_StreamIOCClassDataStructure * SOMLINK resolve_somStream_StreamIOCClassData(void);
				#ifndef somStream_StreamIOCClassData
					#define somStream_StreamIOCClassData    (*(resolve_somStream_StreamIOCClassData()))
				#endif /* somStream_StreamIOCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somestio_Source) || defined(somStream_StreamIO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somestio_Source || somStream_StreamIO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somestio_Source || somStream_StreamIO_Class_Source */
			struct somStream_StreamIOCClassDataStructure SOMDLINK somStream_StreamIOCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somestio_Source) || defined(somStream_StreamIO_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somestio_Source || somStream_StreamIO_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somestio_Source || somStream_StreamIO_Class_Source */
		SOMClass SOMSTAR SOMLINK somStream_StreamIONewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somStream_StreamIO (somStream_StreamIOClassData.classObject)
		#ifndef SOMGD_somStream_StreamIO
			#if (defined(_somStream_StreamIO) || defined(__somStream_StreamIO))
				#undef _somStream_StreamIO
				#undef __somStream_StreamIO
				#define SOMGD_somStream_StreamIO 1
			#else
				#define _somStream_StreamIO _SOMCLASS_somStream_StreamIO
			#endif /* _somStream_StreamIO */
		#endif /* SOMGD_somStream_StreamIO */
		#define somStream_StreamIO_classObj _SOMCLASS_somStream_StreamIO
		#define _SOMMTOKEN_somStream_StreamIO(method) ((somMToken)(somStream_StreamIOClassData.method))
		#ifndef somStream_StreamIONew
			#define somStream_StreamIONew() ( _somStream_StreamIO ? \
				(SOMClass_somNew(_somStream_StreamIO)) : \
				( somStream_StreamIONewClass( \
					somStream_StreamIO_MajorVersion, \
					somStream_StreamIO_MinorVersion), \
				SOMClass_somNew(_somStream_StreamIO))) 
		#endif /* NewsomStream_StreamIO */
		#ifndef somStream_StreamIO_already_streamed
			#define somStream_StreamIO_already_streamed(somSelf,ev,obj,class_obj) \
				SOM_Resolve(somSelf,somStream_StreamIO,already_streamed)  \
					(somSelf,ev,obj,class_obj)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__already_streamed
					#if defined(_already_streamed)
						#undef _already_streamed
						#define SOMGD__already_streamed
					#else
						#define _already_streamed somStream_StreamIO_already_streamed
					#endif
				#endif /* SOMGD__already_streamed */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somStream_StreamIO_already_streamed */
		#ifndef somStream_StreamIO_reset
			#define somStream_StreamIO_reset(somSelf,ev) \
				SOM_Resolve(somSelf,somStream_StreamIO,reset)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__reset
					#if defined(_reset)
						#undef _reset
						#define SOMGD__reset
					#else
						#define _reset somStream_StreamIO_reset
					#endif
				#endif /* SOMGD__reset */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somStream_StreamIO_reset */
		#ifndef somStream_StreamIO_set_buffer
			#define somStream_StreamIO_set_buffer(somSelf,ev,buffer) \
				SOM_Resolve(somSelf,somStream_StreamIO,set_buffer)  \
					(somSelf,ev,buffer)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__set_buffer
					#if defined(_set_buffer)
						#undef _set_buffer
						#define SOMGD__set_buffer
					#else
						#define _set_buffer somStream_StreamIO_set_buffer
					#endif
				#endif /* SOMGD__set_buffer */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somStream_StreamIO_set_buffer */
		#ifndef somStream_StreamIO_get_buffer
			#define somStream_StreamIO_get_buffer(somSelf,ev) \
				SOM_Resolve(somSelf,somStream_StreamIO,get_buffer)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__get_buffer
					#if defined(_get_buffer)
						#undef _get_buffer
						#define SOMGD__get_buffer
					#else
						#define _get_buffer somStream_StreamIO_get_buffer
					#endif
				#endif /* SOMGD__get_buffer */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somStream_StreamIO_get_buffer */
		#ifndef somStream_StreamIO_clear_buffer
			#define somStream_StreamIO_clear_buffer(somSelf,ev) \
				SOM_Resolve(somSelf,somStream_StreamIO,clear_buffer)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__clear_buffer
					#if defined(_clear_buffer)
						#undef _clear_buffer
						#define SOMGD__clear_buffer
					#else
						#define _clear_buffer somStream_StreamIO_clear_buffer
					#endif
				#endif /* SOMGD__clear_buffer */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somStream_StreamIO_clear_buffer */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somStream_StreamIO_somInit SOMObject_somInit
#define somStream_StreamIO_somUninit SOMObject_somUninit
#define somStream_StreamIO_somFree SOMObject_somFree
#define somStream_StreamIO_somGetClass SOMObject_somGetClass
#define somStream_StreamIO_somGetClassName SOMObject_somGetClassName
#define somStream_StreamIO_somGetSize SOMObject_somGetSize
#define somStream_StreamIO_somIsA SOMObject_somIsA
#define somStream_StreamIO_somIsInstanceOf SOMObject_somIsInstanceOf
#define somStream_StreamIO_somRespondsTo SOMObject_somRespondsTo
#define somStream_StreamIO_somDispatch SOMObject_somDispatch
#define somStream_StreamIO_somClassDispatch SOMObject_somClassDispatch
#define somStream_StreamIO_somCastObj SOMObject_somCastObj
#define somStream_StreamIO_somResetObj SOMObject_somResetObj
#define somStream_StreamIO_somPrintSelf SOMObject_somPrintSelf
#define somStream_StreamIO_somDumpSelf SOMObject_somDumpSelf
#define somStream_StreamIO_somDumpSelfInt SOMObject_somDumpSelfInt
#define somStream_StreamIO_somDefaultInit SOMObject_somDefaultInit
#define somStream_StreamIO_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somStream_StreamIO_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somStream_StreamIO_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somStream_StreamIO_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somStream_StreamIO_somDefaultAssign SOMObject_somDefaultAssign
#define somStream_StreamIO_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somStream_StreamIO_somDefaultVAssign SOMObject_somDefaultVAssign
#define somStream_StreamIO_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somStream_StreamIO_somDestruct SOMObject_somDestruct
#define somStream_StreamIO_write_string CosStream_StreamIO_write_string
#define somStream_StreamIO_write_char CosStream_StreamIO_write_char
#define somStream_StreamIO_write_octet CosStream_StreamIO_write_octet
#define somStream_StreamIO_write_unsigned_long CosStream_StreamIO_write_unsigned_long
#define somStream_StreamIO_write_unsigned_short CosStream_StreamIO_write_unsigned_short
#define somStream_StreamIO_write_long CosStream_StreamIO_write_long
#define somStream_StreamIO_write_short CosStream_StreamIO_write_short
#define somStream_StreamIO_write_float CosStream_StreamIO_write_float
#define somStream_StreamIO_write_double CosStream_StreamIO_write_double
#define somStream_StreamIO_write_boolean CosStream_StreamIO_write_boolean
#define somStream_StreamIO_write_object CosStream_StreamIO_write_object
#define somStream_StreamIO_read_string CosStream_StreamIO_read_string
#define somStream_StreamIO_read_char CosStream_StreamIO_read_char
#define somStream_StreamIO_read_octet CosStream_StreamIO_read_octet
#define somStream_StreamIO_read_unsigned_long CosStream_StreamIO_read_unsigned_long
#define somStream_StreamIO_read_unsigned_short CosStream_StreamIO_read_unsigned_short
#define somStream_StreamIO_read_long CosStream_StreamIO_read_long
#define somStream_StreamIO_read_short CosStream_StreamIO_read_short
#define somStream_StreamIO_read_float CosStream_StreamIO_read_float
#define somStream_StreamIO_read_double CosStream_StreamIO_read_double
#define somStream_StreamIO_read_boolean CosStream_StreamIO_read_boolean
#define somStream_StreamIO_read_object CosStream_StreamIO_read_object
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef somStream_MemoryStreamIO_MajorVersion
			#define somStream_MemoryStreamIO_MajorVersion   3
		#endif /* somStream_MemoryStreamIO_MajorVersion */
		#ifndef somStream_MemoryStreamIO_MinorVersion
			#define somStream_MemoryStreamIO_MinorVersion   0
		#endif /* somStream_MemoryStreamIO_MinorVersion */
		typedef struct somStream_MemoryStreamIOClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} somStream_MemoryStreamIOClassDataStructure;
		typedef struct somStream_MemoryStreamIOCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somStream_MemoryStreamIOCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somestio_Source) || defined(somStream_MemoryStreamIO_Class_Source)
				SOMEXTERN struct somStream_MemoryStreamIOClassDataStructure _somStream_MemoryStreamIOClassData;
				#ifndef somStream_MemoryStreamIOClassData
					#define somStream_MemoryStreamIOClassData    _somStream_MemoryStreamIOClassData
				#endif /* somStream_MemoryStreamIOClassData */
			#else
				SOMEXTERN struct somStream_MemoryStreamIOClassDataStructure * SOMLINK resolve_somStream_MemoryStreamIOClassData(void);
				#ifndef somStream_MemoryStreamIOClassData
					#define somStream_MemoryStreamIOClassData    (*(resolve_somStream_MemoryStreamIOClassData()))
				#endif /* somStream_MemoryStreamIOClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somestio_Source) || defined(somStream_MemoryStreamIO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somestio_Source || somStream_MemoryStreamIO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somestio_Source || somStream_MemoryStreamIO_Class_Source */
			struct somStream_MemoryStreamIOClassDataStructure SOMDLINK somStream_MemoryStreamIOClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somestio_Source) || defined(somStream_MemoryStreamIO_Class_Source)
				SOMEXTERN struct somStream_MemoryStreamIOCClassDataStructure _somStream_MemoryStreamIOCClassData;
				#ifndef somStream_MemoryStreamIOCClassData
					#define somStream_MemoryStreamIOCClassData    _somStream_MemoryStreamIOCClassData
				#endif /* somStream_MemoryStreamIOCClassData */
			#else
				SOMEXTERN struct somStream_MemoryStreamIOCClassDataStructure * SOMLINK resolve_somStream_MemoryStreamIOCClassData(void);
				#ifndef somStream_MemoryStreamIOCClassData
					#define somStream_MemoryStreamIOCClassData    (*(resolve_somStream_MemoryStreamIOCClassData()))
				#endif /* somStream_MemoryStreamIOCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somestio_Source) || defined(somStream_MemoryStreamIO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somestio_Source || somStream_MemoryStreamIO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somestio_Source || somStream_MemoryStreamIO_Class_Source */
			struct somStream_MemoryStreamIOCClassDataStructure SOMDLINK somStream_MemoryStreamIOCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somestio_Source) || defined(somStream_MemoryStreamIO_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somestio_Source || somStream_MemoryStreamIO_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somestio_Source || somStream_MemoryStreamIO_Class_Source */
		SOMClass SOMSTAR SOMLINK somStream_MemoryStreamIONewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somStream_MemoryStreamIO (somStream_MemoryStreamIOClassData.classObject)
		#ifndef SOMGD_somStream_MemoryStreamIO
			#if (defined(_somStream_MemoryStreamIO) || defined(__somStream_MemoryStreamIO))
				#undef _somStream_MemoryStreamIO
				#undef __somStream_MemoryStreamIO
				#define SOMGD_somStream_MemoryStreamIO 1
			#else
				#define _somStream_MemoryStreamIO _SOMCLASS_somStream_MemoryStreamIO
			#endif /* _somStream_MemoryStreamIO */
		#endif /* SOMGD_somStream_MemoryStreamIO */
		#define somStream_MemoryStreamIO_classObj _SOMCLASS_somStream_MemoryStreamIO
		#define _SOMMTOKEN_somStream_MemoryStreamIO(method) ((somMToken)(somStream_MemoryStreamIOClassData.method))
		#ifndef somStream_MemoryStreamIONew
			#define somStream_MemoryStreamIONew() ( _somStream_MemoryStreamIO ? \
				(SOMClass_somNew(_somStream_MemoryStreamIO)) : \
				( somStream_MemoryStreamIONewClass( \
					somStream_MemoryStreamIO_MajorVersion, \
					somStream_MemoryStreamIO_MinorVersion), \
				SOMClass_somNew(_somStream_MemoryStreamIO))) 
		#endif /* NewsomStream_MemoryStreamIO */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somStream_MemoryStreamIO_somInit SOMObject_somInit
#define somStream_MemoryStreamIO_somUninit SOMObject_somUninit
#define somStream_MemoryStreamIO_somFree SOMObject_somFree
#define somStream_MemoryStreamIO_somGetClass SOMObject_somGetClass
#define somStream_MemoryStreamIO_somGetClassName SOMObject_somGetClassName
#define somStream_MemoryStreamIO_somGetSize SOMObject_somGetSize
#define somStream_MemoryStreamIO_somIsA SOMObject_somIsA
#define somStream_MemoryStreamIO_somIsInstanceOf SOMObject_somIsInstanceOf
#define somStream_MemoryStreamIO_somRespondsTo SOMObject_somRespondsTo
#define somStream_MemoryStreamIO_somDispatch SOMObject_somDispatch
#define somStream_MemoryStreamIO_somClassDispatch SOMObject_somClassDispatch
#define somStream_MemoryStreamIO_somCastObj SOMObject_somCastObj
#define somStream_MemoryStreamIO_somResetObj SOMObject_somResetObj
#define somStream_MemoryStreamIO_somPrintSelf SOMObject_somPrintSelf
#define somStream_MemoryStreamIO_somDumpSelf SOMObject_somDumpSelf
#define somStream_MemoryStreamIO_somDumpSelfInt SOMObject_somDumpSelfInt
#define somStream_MemoryStreamIO_somDefaultInit SOMObject_somDefaultInit
#define somStream_MemoryStreamIO_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somStream_MemoryStreamIO_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somStream_MemoryStreamIO_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somStream_MemoryStreamIO_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somStream_MemoryStreamIO_somDefaultAssign SOMObject_somDefaultAssign
#define somStream_MemoryStreamIO_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somStream_MemoryStreamIO_somDefaultVAssign SOMObject_somDefaultVAssign
#define somStream_MemoryStreamIO_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somStream_MemoryStreamIO_somDestruct SOMObject_somDestruct
#define somStream_MemoryStreamIO_write_string CosStream_StreamIO_write_string
#define somStream_MemoryStreamIO_write_char CosStream_StreamIO_write_char
#define somStream_MemoryStreamIO_write_octet CosStream_StreamIO_write_octet
#define somStream_MemoryStreamIO_write_unsigned_long CosStream_StreamIO_write_unsigned_long
#define somStream_MemoryStreamIO_write_unsigned_short CosStream_StreamIO_write_unsigned_short
#define somStream_MemoryStreamIO_write_long CosStream_StreamIO_write_long
#define somStream_MemoryStreamIO_write_short CosStream_StreamIO_write_short
#define somStream_MemoryStreamIO_write_float CosStream_StreamIO_write_float
#define somStream_MemoryStreamIO_write_double CosStream_StreamIO_write_double
#define somStream_MemoryStreamIO_write_boolean CosStream_StreamIO_write_boolean
#define somStream_MemoryStreamIO_write_object CosStream_StreamIO_write_object
#define somStream_MemoryStreamIO_read_string CosStream_StreamIO_read_string
#define somStream_MemoryStreamIO_read_char CosStream_StreamIO_read_char
#define somStream_MemoryStreamIO_read_octet CosStream_StreamIO_read_octet
#define somStream_MemoryStreamIO_read_unsigned_long CosStream_StreamIO_read_unsigned_long
#define somStream_MemoryStreamIO_read_unsigned_short CosStream_StreamIO_read_unsigned_short
#define somStream_MemoryStreamIO_read_long CosStream_StreamIO_read_long
#define somStream_MemoryStreamIO_read_short CosStream_StreamIO_read_short
#define somStream_MemoryStreamIO_read_float CosStream_StreamIO_read_float
#define somStream_MemoryStreamIO_read_double CosStream_StreamIO_read_double
#define somStream_MemoryStreamIO_read_boolean CosStream_StreamIO_read_boolean
#define somStream_MemoryStreamIO_read_object CosStream_StreamIO_read_object
#define somStream_MemoryStreamIO_already_streamed somStream_StreamIO_already_streamed
#define somStream_MemoryStreamIO_reset somStream_StreamIO_reset
#define somStream_MemoryStreamIO_set_buffer somStream_StreamIO_set_buffer
#define somStream_MemoryStreamIO_get_buffer somStream_StreamIO_get_buffer
#define somStream_MemoryStreamIO_clear_buffer somStream_StreamIO_clear_buffer
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef somStream_Streamable_MajorVersion
			#define somStream_Streamable_MajorVersion   3
		#endif /* somStream_Streamable_MajorVersion */
		#ifndef somStream_Streamable_MinorVersion
			#define somStream_Streamable_MinorVersion   0
		#endif /* somStream_Streamable_MinorVersion */
		typedef struct somStream_StreamableClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} somStream_StreamableClassDataStructure;
		typedef struct somStream_StreamableCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somStream_StreamableCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somestio_Source) || defined(somStream_Streamable_Class_Source)
				SOMEXTERN struct somStream_StreamableClassDataStructure _somStream_StreamableClassData;
				#ifndef somStream_StreamableClassData
					#define somStream_StreamableClassData    _somStream_StreamableClassData
				#endif /* somStream_StreamableClassData */
			#else
				SOMEXTERN struct somStream_StreamableClassDataStructure * SOMLINK resolve_somStream_StreamableClassData(void);
				#ifndef somStream_StreamableClassData
					#define somStream_StreamableClassData    (*(resolve_somStream_StreamableClassData()))
				#endif /* somStream_StreamableClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somestio_Source) || defined(somStream_Streamable_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somestio_Source || somStream_Streamable_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somestio_Source || somStream_Streamable_Class_Source */
			struct somStream_StreamableClassDataStructure SOMDLINK somStream_StreamableClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somestio_Source) || defined(somStream_Streamable_Class_Source)
				SOMEXTERN struct somStream_StreamableCClassDataStructure _somStream_StreamableCClassData;
				#ifndef somStream_StreamableCClassData
					#define somStream_StreamableCClassData    _somStream_StreamableCClassData
				#endif /* somStream_StreamableCClassData */
			#else
				SOMEXTERN struct somStream_StreamableCClassDataStructure * SOMLINK resolve_somStream_StreamableCClassData(void);
				#ifndef somStream_StreamableCClassData
					#define somStream_StreamableCClassData    (*(resolve_somStream_StreamableCClassData()))
				#endif /* somStream_StreamableCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somestio_Source) || defined(somStream_Streamable_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somestio_Source || somStream_Streamable_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somestio_Source || somStream_Streamable_Class_Source */
			struct somStream_StreamableCClassDataStructure SOMDLINK somStream_StreamableCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somestio_Source) || defined(somStream_Streamable_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somestio_Source || somStream_Streamable_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somestio_Source || somStream_Streamable_Class_Source */
		SOMClass SOMSTAR SOMLINK somStream_StreamableNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somStream_Streamable (somStream_StreamableClassData.classObject)
		#ifndef SOMGD_somStream_Streamable
			#if (defined(_somStream_Streamable) || defined(__somStream_Streamable))
				#undef _somStream_Streamable
				#undef __somStream_Streamable
				#define SOMGD_somStream_Streamable 1
			#else
				#define _somStream_Streamable _SOMCLASS_somStream_Streamable
			#endif /* _somStream_Streamable */
		#endif /* SOMGD_somStream_Streamable */
		#define somStream_Streamable_classObj _SOMCLASS_somStream_Streamable
		#define _SOMMTOKEN_somStream_Streamable(method) ((somMToken)(somStream_StreamableClassData.method))
		#ifndef somStream_StreamableNew
			#define somStream_StreamableNew() ( _somStream_Streamable ? \
				(SOMClass_somNew(_somStream_Streamable)) : \
				( somStream_StreamableNewClass( \
					somStream_Streamable_MajorVersion, \
					somStream_Streamable_MinorVersion), \
				SOMClass_somNew(_somStream_Streamable))) 
		#endif /* NewsomStream_Streamable */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somStream_Streamable_somInit SOMObject_somInit
#define somStream_Streamable_somUninit SOMObject_somUninit
#define somStream_Streamable_somFree SOMObject_somFree
#define somStream_Streamable_somGetClass SOMObject_somGetClass
#define somStream_Streamable_somGetClassName SOMObject_somGetClassName
#define somStream_Streamable_somGetSize SOMObject_somGetSize
#define somStream_Streamable_somIsA SOMObject_somIsA
#define somStream_Streamable_somIsInstanceOf SOMObject_somIsInstanceOf
#define somStream_Streamable_somRespondsTo SOMObject_somRespondsTo
#define somStream_Streamable_somDispatch SOMObject_somDispatch
#define somStream_Streamable_somClassDispatch SOMObject_somClassDispatch
#define somStream_Streamable_somCastObj SOMObject_somCastObj
#define somStream_Streamable_somResetObj SOMObject_somResetObj
#define somStream_Streamable_somPrintSelf SOMObject_somPrintSelf
#define somStream_Streamable_somDumpSelf SOMObject_somDumpSelf
#define somStream_Streamable_somDumpSelfInt SOMObject_somDumpSelfInt
#define somStream_Streamable_somDefaultInit SOMObject_somDefaultInit
#define somStream_Streamable_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somStream_Streamable_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somStream_Streamable_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somStream_Streamable_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somStream_Streamable_somDefaultAssign SOMObject_somDefaultAssign
#define somStream_Streamable_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somStream_Streamable_somDefaultVAssign SOMObject_somDefaultVAssign
#define somStream_Streamable_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somStream_Streamable_somDestruct SOMObject_somDestruct
#define somStream_Streamable__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define somStream_Streamable_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#define somStream_Streamable_reinit somOS_ServiceBase_reinit
#define somStream_Streamable_capture somOS_ServiceBase_capture
#define somStream_Streamable_GetInstanceManager somOS_ServiceBase_GetInstanceManager
#define somStream_Streamable_init_for_object_creation somOS_ServiceBase_init_for_object_creation
#define somStream_Streamable_init_for_object_reactivation somOS_ServiceBase_init_for_object_reactivation
#define somStream_Streamable_init_for_object_copy somOS_ServiceBase_init_for_object_copy
#define somStream_Streamable_uninit_for_object_move somOS_ServiceBase_uninit_for_object_move
#define somStream_Streamable_uninit_for_object_passivation somOS_ServiceBase_uninit_for_object_passivation
#define somStream_Streamable_uninit_for_object_destruction somOS_ServiceBase_uninit_for_object_destruction
#define somStream_Streamable_externalize_to_stream CosStream_Streamable_externalize_to_stream
#define somStream_Streamable_internalize_from_stream CosStream_Streamable_internalize_from_stream
#define somStream_Streamable__get_external_form_id CosStream_Streamable__get_external_form_id
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somestio_Header_h */
