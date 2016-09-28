/*
 * $Header$
 */

/************************************************************************/
/*                       Linux partition filter.                        */
/*          (C) Copyright Deon van der Westhuysen, July 1995.           */
/*                                                                      */
/*  Dedicated to Jesus Christ, my Lord and Saviour.                     */
/*                                                                      */
/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2, or (at your option)  */
/* any later version.                                                   */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this program; if not, write to the Free Software          */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */
/*                                                                      */
/*  This code is still under development; expect some rough edges.      */
/*                                                                      */
/************************************************************************/

#include "e2data.h"

#ifdef __WATCOMC__
#pragma data_seg ( "_DATA", "DATA" ) ;
#endif

PFN		DevHelp= NULL;		/* Ptr to DevHelp service routine */
USHORT   ADDHandle= 0;		/* Our adapter device driver handle */
void far *	pDataSeg= NULL;		/* Virtual address of data segment */
ULONG		ppDataSeg= 0L;		/* Physical address of data segment */

int		InstallFlags= 0;
TBaseUnitRec	BaseUnits[MAX_LINUX_PARTITIONS]= {0};
int		NumBaseUnits= 0;
TVirtUnitRec	VirtUnits[MAX_LINUX_PARTITIONS]= {0};
int		NumVirtUnits= 0;
USHORT	MountTable[MAX_LINUX_PARTITIONS]= {0};
int		MountCount= 0;
BYTE     rgfFakePart[256] = {0};

#ifdef __WATCOMC__
#pragma data_seg ( ) ;
#endif
