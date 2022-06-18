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

#define LName_Class_Source

#include <rhbsomnm.h>

#include <lname.ih>

SOM_Scope unsigned long SOMLINK lname_num_components(
	LName SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope LNameComponent SOMSTAR SOMLINK lname_get_component(
	LName SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long i)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i)

	return 0;
}

SOM_Scope CosNaming_Name SOMLINK lname_to_idl_form(
	LName SOMSTAR somSelf,
	Environment *ev)
{
	CosNaming_Name name={0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return name;
}

SOM_Scope void SOMLINK lname_from_idl_form(
	LName SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
}

SOM_Scope void SOMLINK lname_destroy(
	LName SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope boolean SOMLINK lname_less_than(
	LName SOMSTAR somSelf,
	Environment *ev,
	/* in */ LName SOMSTAR ln)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(ln)

	return 0;
}

SOM_Scope LNameComponent SOMSTAR SOMLINK lname_delete_component(
	LName SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long i)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i)

	return 0;
}


SOM_Scope boolean SOMLINK lname_equal(
	LName SOMSTAR somSelf,
	Environment *ev,
	/* in */ LName SOMSTAR ln)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(ln)

	return 0;
}

SOM_Scope LName SOMSTAR SOMLINK lname_insert_component(
	LName SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long i,
	/* in */ LNameComponent SOMSTAR n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(i)

	return 0;
}




