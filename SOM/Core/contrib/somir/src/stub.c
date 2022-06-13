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

#define Repository_Class_Source
#define M_Repository_Class_Source
#define InterfaceDef_Class_Source
#define OperationDef_Class_Source
#define Contained_Class_Source
#define Container_Class_Source
#define ModuleDef_Class_Source
#define AttributeDef_Class_Source
#define ParameterDef_Class_Source
#define ConstantDef_Class_Source
#define ExceptionDef_Class_Source
#define TypeDef_Class_Source

#include <rhbsomex.h>
#include <stdio.h>

#ifdef SOMDLLEXPORT
	#define SOM_IMPORTEXPORT_somir SOMDLLEXPORT
#else
	#define SOM_IMPORTEXPORT_somir 
#endif

#include <somstubs.h>
#include <somir.h>
#include <containd.h>
#include <containr.h>
#include <repostry.h>
#include <constdef.h>
#include <attribdf.h>
#include <paramdef.h>
#include <excptdef.h>
#include <operatdf.h>
#include <intfacdf.h>
#include <typedef.h>
#include <moduledf.h>
#include <rhbsomex.h>

SOMSTUB(Repository)
SOMSTUB(M_Repository)
SOMSTUB(Contained)
SOMSTUB(Container)
SOMSTUB(InterfaceDef)
SOMSTUB(ConstantDef)
SOMSTUB(AttributeDef)
SOMSTUB(ExceptionDef)
SOMSTUB(ParameterDef)
SOMSTUB(TypeDef)
SOMSTUB(OperationDef)
SOMSTUB(ModuleDef)

char * SOMLINK irGetFileNames(void) { return NULL; }
char * SOMLINK irGetLastFileName(void) { return NULL; }

SOMInitModule_begin(somir)

    char *envn="DLLPATHENV";
    char *env=getenv(envn);
    char *p;

    if (!env) 
	{
		env="LD_LIBRARY_PATH";
	}

    p=getenv(env);

    fprintf(stderr,"SOMSTUBS::%s,%d\n",__FILE__,__LINE__);

    if (p) 
	{
		fprintf(stderr,"%s=%s\n",env,p);
	}

SOMInitModule_end
