/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: Bad_Block_Relocation.h
 */

/*
 * Change History:
 *
 */



#ifndef  BBR_MANAGER

#define BBR_MANAGER

#include "Bad_Block_Relocation_Feature.h"

#include "engine.h"

#include "lvm_plug.h"

void BBR_Get_Required_LVM_Version( CARDINAL32 * Major_Version_Number, CARDINAL32 * Minor_Version_Number);

ADDRESS BBR_Exchange_Function_Tables( ADDRESS Common_Services );

#endif

