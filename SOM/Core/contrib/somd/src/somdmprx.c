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
#include <rhbsomd.h>

#ifndef SOM_Module_somdmprx_Source
#define SOM_Module_somdmprx_Source
#endif
#define SOMDMetaproxy_Class_Source

#include "somdmprx.ih"

#ifdef SOMDMetaproxy_parent_SOMClass_somInitMIClass
SOM_Scope void  SOMLINK somdmprx_somInitMIClass(SOMDMetaproxy SOMSTAR somSelf, 
                                                unsigned long inherit_vars, 
                                                corbastring className, 
                                                SOMClass_SOMClassSequence* parentClasses, 
                                                long dataSize, 
                                                long dataAlignment, 
                                                long maxStaticMethods, 
                                                long majorVersion, 
                                                long minorVersion)
{
   SOMDMetaproxy_parent_SOMClass_somInitMIClass(somSelf, inherit_vars, 
                                                 className, parentClasses, 
                                                 dataSize, dataAlignment, 
                                                 maxStaticMethods, 
                                                 majorVersion, 
                                                 minorVersion);
}
#endif

