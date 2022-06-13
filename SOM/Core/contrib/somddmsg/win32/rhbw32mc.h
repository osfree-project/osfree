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

#ifdef RC_INVOKED
#else
	#include <somddmsg.h>
	#define IDM_DISPLAY_NAME		SOMDDMSG_DisplayName
	#define RHBW32MC_OpenLog        SOMDDMSG_OpenLog
	#define RHBW32MC_WriteLogA      SOMDDMSG_WriteLogA
	#define RHBW32MC_WriteLogW      SOMDDMSG_WriteLogW
	#define RHBW32MC_CloseLog       SOMDDMSG_CloseLog
#endif

#define IDS_APPLICATION		1
