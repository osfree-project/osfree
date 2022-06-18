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

#define SOM_DLL_somdcomm

#include <rhbopt.h>
#ifdef _WIN32
#	include <windows.h>
#endif
#include <rhbsomex.h>
#include <somd.h>
#include <rhbsomut.h>

#define SOM_Module_formarsh_Source

#include <formarsh.h>

#if defined(_PLATFORM_MACINTOSH_) && GENERATINGCFM
RHBOPT_CFM_init(SOMDCOMM,initBlockPtr)
{
	if (initBlockPtr)
	{
/*		RHB_NewClass(SOMDForeignMarshaler);*/
		return 0;
	}
	return -1;
}
#endif

SOMInitModule_begin(somdcomm)

	SOMInitModule_new(SOMDForeignMarshaler);

SOMInitModule_end

static void SOMLINK SOMD_FreeType_object(SOMCDR_unmarshal_filter *filter,Environment *ev,void *_value,TypeCode _type)
{
	switch (TypeCode_kind(_type,ev))
	{
	case tk_objref:
	case tk_Principal:
		{
			SOMObject SOMSTAR *ho=_value;
			SOMObject SOMSTAR val=*ho;
			ho[0]=NULL;
			if (val) 
			{
				SOMObject_somFree(val);
			}
		}
		break;
	case tk_foreign:
		RHBCDR_call_foreign_handler(
				ev,
				NULL,
				_value,
				_type,
				SOMD_OpFreeStorage);
		break;
	}
}

#ifdef SOMDLLEXPORT
SOMDLLEXPORT
#endif
void SOMLINK SOMD_FreeType(Environment * ev,void *ptr,TypeCode tc)
{
	RHBOPT_ASSERT(ev)
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (ev && (ev->_major==NO_EXCEPTION)) 
	{
		SOMCDR_unmarshal_filter filter={NULL,SOMD_FreeType_object,NULL};

#ifdef RHBCDR_free_data
#	undef RHBCDR_free_data
#endif

		RHBCDR_free_data(ev,&filter,tc,ptr);
	}

}

#ifdef _WIN32
BOOL CALLBACK DllMain(HMODULE hMod,DWORD dw,LPVOID pv)
{
	switch (dw)
	{
	case DLL_PROCESS_ATTACH:
		return DisableThreadLibraryCalls(hMod);
	}

	return 1;
}
#endif
