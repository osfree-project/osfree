/**************************************************************************
 *
 *  Copyright 1998-2010, Roger Brown
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rhbsc.h>
/* #include <rhbsctyp.h>*/

void RHBbase_type::boot_root_types(RHBPreprocessor *pp,RHBrepository *rp)
{
	RHBtype *t;
	RHBtype *int_type;
	RHBtype *double_type;

	int_type=new RHBbase_type(pp,"int",0,0,0);

	double_type=new RHBbase_type(pp,"double",0,0,1);

	t=new RHBbase_type(pp,"long",0,0,0);

	rp->add(pp,t);

	t=new RHBbase_type(pp,
			"short",int_type,0,0);

	rp->add(pp,t);

	t=new RHBbase_type(pp,"char",int_type,0,0);
	rp->add(pp,t);

	t=new RHBbase_type(pp,"octet",int_type,0,0);
	rp->add(pp,t);

	t=new RHBbase_type(pp,"boolean",int_type,0,0);
	rp->add(pp,t);

	t=new RHBbase_type(pp,"float",double_type,0,1);
	rp->add(pp,t);

	t=new RHBbase_type(pp,"double",double_type,0,1);
	rp->add(pp,t);

	t=new RHBbase_type(pp,"void",0,1,0);
	rp->add(pp,t);

/*t=new RHBbase_type(pp,"string",0,0,0);*/
	t=new RHBstring_type(pp,"string");
	rp->add(pp,t);

	t=new RHBany_type(pp,"any",0,1);
	rp->add(pp,t);

	t=new RHBTypeCode_type(pp,"TypeCode",0,0);
	rp->add(pp,t);

	t=new RHBinterface(pp,"SOMObject");

	rp->add(pp,t);

	/*RHBelement *el= */rp->children();
}

RHBattribute * RHBelement::is_attribute()
{
	return 0;
}

RHBattribute * RHBattribute::is_attribute()
{
	return this;
}

long RHBvalue::read_hex_value(const char *p)
{
	long x=0;

	while (*p)
	{
		long d=0;

		switch (*p)
		{
		case '0': d=0; break;
		case '1': d=1; break;
		case '2': d=2; break;
		case '3': d=3; break;
		case '4': d=4; break;
		case '5': d=5; break;
		case '6': d=6; break;
		case '7': d=7; break;
		case '8': d=8; break;
		case '9': d=9; break;
		case 'A': 
		case 'a': d=10; break;
		case 'B': 
		case 'b': d=11; break;
		case 'C': 
		case 'c': d=12; break;
		case 'D': 
		case 'd': d=13; break;
		case 'E': 
		case 'e': d=14; break;
		case 'F': 
		case 'f': d=15; break;
		default: bomb("invalid char");
		}

		x<<=4;
		x+=d;

		p++;
	}

	return x;
}

long RHBvalue::read_octal_value(const char *p)
{
	long x=0;

	while (*p)
	{
		long d=0;

		switch (*p)
		{
		case '0': d=0; break;
		case '1': d=1; break;
		case '2': d=2; break;
		case '3': d=3; break;
		case '4': d=4; break;
		case '5': d=5; break;
		case '6': d=6; break;
		case '7': d=7; break;
		default: bomb("invalid char");
		}

		x<<=3;
		x+=d;

		p++;
	}

	return x;
}

