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

#include <rhbseh.h>

#ifdef _WIN32
#	include <windows.h>
#endif

#ifdef _WIN32
static int __stdcall rhbseh_guardian_except(
		struct rhbseh_guardian *g,
		void *pv)
{
	PEXCEPTION_POINTERS pex=pv;
	PEXCEPTION_RECORD per=pex->ExceptionRecord;

	if (!(per->ExceptionFlags & EXCEPTION_NONCONTINUABLE))
	{
		switch (per->ExceptionCode)
		{
		case ERROR_OPERATION_ABORTED: /* 995 */
			g->except_code=pex->ExceptionRecord->ExceptionCode;
			g->reraise_flag=1;
			return EXCEPTION_CONTINUE_EXECUTION;

		default:
			break;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

static void __stdcall rhbseh_guardian_final(
		struct rhbseh_guardian *g)
{
	if (g->reraise_flag)
	{
		g->reraise_flag=0;
		RaiseException(g->except_code,0,0,0);
	}
}

static struct rhbseh_guardian_vtbl rhbseh_guardian_vtbl={
	rhbseh_guardian_except,
	rhbseh_guardian_final
};
void __stdcall rhbseh_guardian(struct rhbseh_guardian *g)
{
	g->except_code=0;
	g->reraise_flag=0;
	g->vtbl=&rhbseh_guardian_vtbl;
}
#endif
