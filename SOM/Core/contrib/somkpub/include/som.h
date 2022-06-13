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

/*********************************************************************
 * main header file
 *
 */

#ifndef som_h
#define som_h

#include <stdlib.h>
#include <somnames.h>
#include <somplatf.h>
#include <somtypes.h>
#include <somcdev.h>
#include <somcorba.h>
#include <somapi.h>

#ifndef SOM_NO_OBJECTS
#	ifndef SOMObject_VA_EXTERN
		#define SOMObject_VA_EXTERN
#	endif

#	ifdef __cplusplus
#		include <somobj.xh>
#		include <somcls.xh>
#		include <somcm.xh>
#	else
#		include <somobj.h>
#		include <somcls.h>
#		include <somcm.h>
#	endif
#endif /* SOM_NO_OBJECTS */

#endif /* som_h */
