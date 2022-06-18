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

RHBapi_emitter::RHBapi_emitter(RHBrepository *rep)
:
/*RHBemitter(rep),*/
RHBheader_emitter(rep)
{
}

boolean RHBapi_emitter::generate(RHBoutput *out,char const *f)
{
	out_printf(out,"/* %s for %s */\n",__FILE__,f);

	idl_name=f;

	cplusplus=1;
	internal=0;

	preflight_macros_from_idl_filename(f);

	generate_classes(out,GetRepository(),0,1);

	return 1;
}
