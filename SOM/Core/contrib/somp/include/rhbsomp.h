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

#ifdef _WIN32
/*	#include <windows.h>*/
#endif

#define SOM_Module_defedidl_Source
#define SOM_Module_eda_Source
#define SOM_Module_fma_Source
#define SOM_Module_fmi_Source
#define SOM_Module_fmib_Source
#define SOM_Module_fsagm_Source
#define SOM_Module_fsgm_Source
#define SOM_Module_iogma_Source
#define SOM_Module_iogrp_Source
#define SOM_Module_mia_Source
#define SOM_Module_nsma_Source
#define SOM_Module_objset_Source
#define SOM_Module_psma_Source
#define SOM_Module_pid_Source
#define SOM_Module_po_Source
#define SOM_Module_poid_Source
#define SOM_Module_poida_Source
#define SOM_Module_srkset_Source

#include <rhbsomex.h>
#ifdef USE_APPLE_SOM
#	include <som.h>
#else
#	include <somref.h>
#endif
#include <snglicls.h>
#include <somsid.h>
/* #include <somp.h>*/
#include <sompstad.h>
/*#include <sompstat.h>*/

#include <somir.h>
#include <defedidl.h>
#include <eda.h>
#include <fma.h>
#include <fmi.h>
#include <fmib.h>
#include <fsagm.h>
#include <fsgm.h>
#include <iogma.h>
#include <iogrp.h>
#include <mia.h>
#include <nsma.h>
#include <objset.h>
#include <srkset.h>
#include <po.h>
#include <pid.h>
#include <poid.h>
#include <poida.h>
#include <psma.h>

