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

#ifndef __RHBUNION_H__
#define __RHBUNION_H__

#ifdef _PLATFORM_MACINTOSH_
	#if powerc
		#pragma options align=mac68k
	#endif  /* powerc */
#endif /* _PLATFORM_MACINTOSH_ */

struct RHBUNION_min_align
{
	octet _d;
	union
	{
		octet o;
	} _u;
};

#ifdef _PLATFORM_MACINTOSH_
	#if powerc
		#pragma options align=reset
	#endif  /* powerc */
#endif /* _PLATFORM_MACINTOSH_ */


static any RHBUNION_get_switch(Environment *ev,TypeCode tc,void *pv)
{
	any result={NULL,NULL};
	if (!ev->_major)
	{
		if (TypeCode_kind(tc,ev) != tk_union) 
		{
			RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,MAYBE);
		}
		else
		{
			if (!ev->_major)
			{
				any type=TypeCode_parameter(tc,ev,1);

				if (!ev->_major)

				/* the switch value leads the union/struct */

				result._value=pv;
				result._type=*((TypeCode *)type._value);
			}
		}
	}

	return result;
}

static long RHBUNION_cast_any_to_long(Environment *ev,any *a)
{
#define RHBUNION_cast_any_to_long_case(x,y)  case x: return (long)*((y *)a->_value)

	switch (TypeCode_kind(a->_type,ev))
	{
	RHBUNION_cast_any_to_long_case(tk_long,long);
	RHBUNION_cast_any_to_long_case(tk_short,short);
	RHBUNION_cast_any_to_long_case(tk_ulong,unsigned long);
	RHBUNION_cast_any_to_long_case(tk_ushort,unsigned short);
	RHBUNION_cast_any_to_long_case(tk_enum,TCKind);
	RHBUNION_cast_any_to_long_case(tk_octet,octet);
	}

#undef RHBUNION_cast_any_to_long_case

	return 0;
}

static any RHBUNION_get_value(Environment *ev,TypeCode tc,void *pv)
{
	any result={NULL,NULL};
	any union_switch=RHBUNION_get_switch(ev,tc,pv);

	if (!ev->_major)
	{
		octet *op=pv;
		int default_desc=-1;
		int desc_selected=-1;
		size_t pos=(int)(size_t)&(((struct RHBUNION_min_align *)NULL)->_u);
		size_t switch_size=TypeCode_size(union_switch._type,ev);
		int j=2,k=0;
		int params=TypeCode_param_count(tc,ev);
		long switch_value=RHBUNION_cast_any_to_long(ev,&union_switch);

		/* how big '_d' item actually is */
		if (switch_size > pos) pos=switch_size;

		while (j < params)
		{
			long tc_case=TCREGULAR_CASE;
			any b=TypeCode_parameter(tc,ev,j);
			long value=RHBUNION_cast_any_to_long(ev,&b);
			any c=TypeCode_parameter(tc,ev,j+2);
			TypeCode t=*((TypeCode *)c._value);
			unsigned short align=(unsigned short)TypeCode_alignment(t,ev);

			if (TypeCode_kind(b._type,ev)==tk_octet)
			{
				octet *op=b._value;
				if (op[0]==0) tc_case=TCDEFAULT_CASE;
			}

			if (tc_case==TCDEFAULT_CASE)
			{
				default_desc=k;
			}

			if (value==switch_value)
			{
				desc_selected=k;
			}

			if (align > pos)
			{
				pos=align;
			}

			j+=3;
			k++;
		}

		if (desc_selected==-1)
		{
			if (default_desc==-1)
			{
				RHBOPT_throw_StExcep(ev,DATA_CONVERSION,Boundary,MAYBE);
			}
			else
			{
				desc_selected=default_desc;
			}
		}

		/* 2 to get past name and swTC, 
			*3, as that many entries per element
			+2 as it's the third param */

		if (!ev->_major)
		{
			any actual=TypeCode_parameter(tc,ev,
					2+(desc_selected *3)+2);

			result._type=*((TypeCode *)actual._value);
			result._value=op+pos;
		}
	}

	return result;
}
#endif
