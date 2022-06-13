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

#include <rhbopt.h>

#include <rhbsomex.h>

#ifdef USE_APPLE_SOM
#	include <som.h>
#else
#	include <somref.h>
#endif

#include <naming.h>
#include <omgidobj.h>
#include <stexcep.h>
#include <somderr.h>

#ifndef SOM_Module_omgestio_Source
	#define SOM_Module_omgestio_Source
#endif

#define CosStream_Streamable_Class_Source
#define CosStream_StreamIO_Class_Source
#define CosStream_StreamableFactory_Class_Source

#include <omgestio.ih>

static void throw_no_implement(Environment *ev);
static void throw_no_implement(Environment *ev)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
}

SOM_Scope void SOMLINK somestrm_CosStream_Streamable_externalize_to_stream(
	CosStream_Streamable SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosStream_StreamIO SOMSTAR stream)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(stream);
	throw_no_implement(ev);
}

SOM_Scope void SOMLINK somestrm_CosStream_Streamable_internalize_from_stream(
	CosStream_Streamable SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosStream_StreamIO SOMSTAR stream,
	/* in */ CosLifeCycle_FactoryFinder SOMSTAR ff)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(stream);
	SOM_IgnoreWarning(ff);
	throw_no_implement(ev);
}

SOM_Scope CosObjectIdentity_ObjectIdentifier SOMLINK somestrm_CosStream_Streamable__get_constant_random_id(
	CosStream_Streamable SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	return 0;
}


SOM_Scope boolean SOMLINK somestrm_CosStream_Streamable_is_identical(
	CosStream_Streamable SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosObjectIdentity_IdentifiableObject SOMSTAR other_object)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(other_object);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope void SOMLINK somestrm_CosStream_Streamable_somInit(
	CosStream_Streamable SOMSTAR somSelf)
{
	CosStream_Streamable_parent_CosObjectIdentity_IdentifiableObject_somInit(somSelf);
}

SOM_Scope void SOMLINK somestrm_CosStream_Streamable_somUninit(
	CosStream_Streamable SOMSTAR somSelf)
{
	CosStream_Streamable_parent_CosObjectIdentity_IdentifiableObject_somUninit(somSelf);
}

SOM_Scope boolean SOMLINK somestrm_CosStream_StreamIO_read_boolean(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	return 0;
}

SOM_Scope double SOMLINK somestrm_CosStream_StreamIO_read_double(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0.0;
}

SOM_Scope corbastring SOMLINK somestrm_CosStream_StreamIO_read_string(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope char SOMLINK somestrm_CosStream_StreamIO_read_char(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope octet SOMLINK somestrm_CosStream_StreamIO_read_octet(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope long SOMLINK somestrm_CosStream_StreamIO_read_long(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope short SOMLINK somestrm_CosStream_StreamIO_read_short(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope unsigned long SOMLINK somestrm_CosStream_StreamIO_read_unsigned_long(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope unsigned short SOMLINK somestrm_CosStream_StreamIO_read_unsigned_short(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}

SOM_Scope float SOMLINK somestrm_CosStream_StreamIO_read_float(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return (float)0.0;
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_string(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring item)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(item);
	throw_no_implement(ev);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_char(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ char item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_object(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosStream_Streamable SOMSTAR item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_short(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ short item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_boolean(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_double(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ double item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_octet(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ octet item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_float(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ float item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_long(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ long item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_unsigned_short(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope void SOMLINK somestrm_CosStream_StreamIO_write_unsigned_long(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long item)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(item);
}

SOM_Scope CosStream_Streamable SOMSTAR SOMLINK somestrm_CosStream_StreamIO_read_object(
	CosStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosLifeCycle_FactoryFinder SOMSTAR ff,
	/* in */ CosStream_Streamable SOMSTAR obj)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);
	SOM_IgnoreWarning(ff);
	SOM_IgnoreWarning(obj);

	return 0;
}

SOM_Scope CosLifeCycle_Key SOMLINK somestrm_CosStream_Streamable__get_external_form_id(
	CosStream_Streamable SOMSTAR somSelf,
	Environment *ev)
{
	CosLifeCycle_Key key;

	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	memset(&key,0,sizeof(key));
	return key;
}

SOM_Scope CosStream_Streamable SOMSTAR SOMLINK somestrm_CosStream_StreamableFactory_create_uninitialized(
	CosStream_StreamableFactory SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_no_implement(ev);

	return 0;
}


