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

#ifndef __SOMTC__
#define __SOMTC__

#ifdef __cplusplus
	#include <som.xh>
#else
	#include <som.h>
#endif

#ifndef SOM_IMPORTEXPORT_somtc
	#ifdef SOMDLLIMPORT
		#define SOM_IMPORTEXPORT_somtc  SOMEXTERN SOMDLLIMPORT
	#else
		#define SOM_IMPORTEXPORT_somtc  SOMEXTERN
	#endif
#endif /* SOM_IMPORTEXPORT_somtc */

#ifndef _IDL_SEQUENCE_string_defined
	#define _IDL_SEQUENCE_string_defined
	SOM_SEQUENCE_TYPEDEF(string);
#endif /* _IDL_SEQUENCE_string_defined */

#define TCREGULAR_CASE  1L
#define TCDEFAULT_CASE  2L

SOM_IMPORTEXPORT_somtc void     SOMLINK tcPrint(TypeCode t,Environment *ev);
SOM_IMPORTEXPORT_somtc short    SOMLINK tcAlignment (TypeCode t,Environment *ev);
SOM_IMPORTEXPORT_somtc TypeCode SOMLINK tcCopy(TypeCode tc,Environment *ev);
SOM_IMPORTEXPORT_somtc boolean  SOMLINK tcEqual(TypeCode tc,Environment *ev,TypeCode t);
SOM_IMPORTEXPORT_somtc void     SOMLINK tcFree(TypeCode tc,Environment *ev);
SOM_IMPORTEXPORT_somtc TCKind   SOMLINK tcKind(TypeCode tc,Environment *ev);
SOM_IMPORTEXPORT_somtc long     SOMLINK tcParmCount(TypeCode tc,Environment *ev);
SOM_IMPORTEXPORT_somtc any      SOMLINK tcParameter(TypeCode tc,Environment *ev,long index);
SOM_IMPORTEXPORT_somtc void     SOMLINK tcSetAlignment(TypeCode t,Environment *ev, short a);
SOM_IMPORTEXPORT_somtc long     SOMLINK tcSize(TypeCode t,Environment *ev);
SOM_IMPORTEXPORT_somtc TypeCode SOMLINK tcNewVL (TCKind tag, va_list ap);
SOM_IMPORTEXPORT_somtc GENERIC_SEQUENCE * SOMLINK tcSequenceNew (TypeCode t,unsigned long max);
SOM_IMPORTEXPORT_somtc TypeCode SOMLINK tcNew(TCKind tag, ...);
SOM_IMPORTEXPORT_somtc long     SOMLINK tcSetZeroOriginEnum(TypeCode t, Environment *ev, boolean value);
SOM_IMPORTEXPORT_somtc boolean  SOMLINK tcGetZeroOriginEnum(TypeCode t, Environment *ev);	
SOM_IMPORTEXPORT_somtc _IDL_SEQUENCE_string SOMLINK tcSeqFromListString(const char *str);

#define TypeCodeNew						tcNew
#define TypeCodeNewVL(tag,ap)			tcNewVL(tag,ap)
#define TypeCode_kind(tc,ev)			tcKind(tc,ev)
#define TypeCode_param_count(tc,ev)		tcParmCount(tc,ev)
#define TypeCode_parameter(tc,ev,i)		tcParameter(tc,ev,i)
#define TypeCode_size(tc,ev)			tcSize(tc,ev)
#define TypeCode_alignment(tc,ev)		tcAlignment(tc,ev)
#define TypeCode_free(tc,ev)			tcFree(tc,ev)
#define TypeCode_setAlignment(tc,ev,a)	tcSetAlignment(tc,ev,a)
#define TypeCode_copy(tc,ev)			tcCopy(tc,ev)
#define TypeCode_print(tc,ev)			tcPrint(tc,ev)
#define TypeCode_equal(tc,ev,tc2)		tcEqual(tc,ev,tc2)
#define TypeCode_setZeroOriginEnum		tcSetZeroOriginEnum
#define TypeCode_getZeroOriginEnum		tcGetZeroOriginEnum

/* somVaBuf support */

#ifndef SOMVABUF
	#define SOMVABUF
	typedef somToken somVaBuf;

	SOM_IMPORTEXPORT_somtc void * SOMLINK somVaBuf_create(char *, int);
	SOM_IMPORTEXPORT_somtc void   SOMLINK somVaBuf_get_valist(void *, va_list *);
	SOM_IMPORTEXPORT_somtc void   SOMLINK somVaBuf_destroy(void *);
	SOM_IMPORTEXPORT_somtc long   SOMLINK somVaBuf_add(void * , void *, int);
	SOM_IMPORTEXPORT_somtc void * SOMLINK somvalistGetTarget(va_list);
	SOM_IMPORTEXPORT_somtc void   SOMLINK somvalistSetTarget(va_list, void* );
#endif /* SOMVABUF */

#ifdef PRAGMA_IMPORT_SUPPORTED
	#if PRAGMA_IMPORT_SUPPORTED
		#ifdef BUILD_SOMTC
			#pragma export list tcAlignment,tcNew,tcCopy,tcNewVL, \
				tcFree,tcKind,tcParmCount,tcParameter,tcSetZeroOriginEnum, \
				tcPrint,tcSetAlignment,tcSize,tcEqual,tcSequenceNew,tcGetZeroOriginEnum,\
				somVaBuf_create,somVaBuf_get_valist,somVaBuf_destroy,\
				somVaBuf_add,somvalistGetTarget,somvalistSetTarget
		#else
			#pragma import list tcAlignment,tcNew,tcCopy,tcNewVL, \
				tcFree,tcKind,tcParmCount,tcParameter,tcSetZeroOriginEnum, \
				tcPrint,tcSetAlignment,tcSize,tcEqual,tcSequenceNew,tcGetZeroOriginEnum,\
				somVaBuf_create,somVaBuf_get_valist,somVaBuf_destroy,\
				somVaBuf_add,somvalistGetTarget,somvalistSetTarget
		#endif
	#endif
#endif /* PRAGMA_IMPORT_SUPPORTED */

#endif /* __SOMTC__ */

