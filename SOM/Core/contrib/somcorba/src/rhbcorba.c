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
#include <rhbcorba.h>
#include <corba.h>

SOMInitModule_begin(somcorba)

	SOMInitModule_new(CORBA_DataInputStream);
	SOMInitModule_new(CORBA_DataOutputStream);
	SOMInitModule_new(CORBA_Policy);
	SOMInitModule_new(CORBA_Object);
	SOMInitModule_new(CORBA_Current);
	SOMInitModule_new(CORBA_Repository);
	SOMInitModule_new(CORBA_InterfaceDef);
	SOMInitModule_new(CORBA_OperationDef);
	SOMInitModule_new(CORBA_AttributeDef);
	SOMInitModule_new(CORBA_ExceptionDef);
	SOMInitModule_new(CORBA_ModuleDef);

SOMInitModule_end

#if defined(_PLATFORM_MACINTOSH_) && GENERATINGCFM
void SOMCORBA_CFM_term(void);
RHBOPT_CFM_init(SOMCORBA,initBlockPtr)
{
	return 0;
}
void SOMCORBA_CFM_term(void)
{
}
#endif
