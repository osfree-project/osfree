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
/* code generater for formarsh.idl */
#include <rhbsomex.h>
#include <somd.h>
#define SOM_Module_formarsh_Source
#define SOMDForeignMarshaler_Class_Source
#include <formarsh.ih>
/* overridden methods for ::SOMDForeignMarshaler */
/* introduced methods for ::SOMDForeignMarshaler */
/* introduced method ::SOMDForeignMarshaler::marshal */
SOM_Scope void SOMLINK formarsh_marshal(
	SOMDForeignMarshaler SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring latent_param,
	/* in */ somToken dataPtr,
	/* in */ SOMD_marshaling_direction_t direction,
	/* in */ SOMD_marshaling_op_t function,
	/* in */ CosStream_StreamIO SOMSTAR streamio)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
}
