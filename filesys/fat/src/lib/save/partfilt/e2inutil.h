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

#ifndef _E2INUTIL_H_
#define _E2INUTIL_H_

#include "os2head.h"

USHORT InitAddBaseUnit (PADDEntryPoint pADDEntry, PADAPTERINFO pAdapterInfo);
USHORT InitRemoveBaseUnit (void);
USHORT InitFilterBaseUnit (int Index);
int InitAddVirtualUnit (ULONG StartRBA, ULONG NumSectors, UCHAR SysIndicator);

USHORT InitReadDevTable (PADDEntryPoint pAddEP, PDEVICETABLE pDevTable,
                         USHORT DevTableLen);
USHORT InitAllocateUnit (PADDEntryPoint pAddEP, USHORT UnitHandle);
USHORT InitFreeUnit (PADDEntryPoint pAddEP, USHORT UnitHandle);
USHORT InitChangeUnitInfo (PADDEntryPoint pAddEP, USHORT UnitHandle,
                           PUNITINFO pUnitInfo);
USHORT InitGetGeometry (PADDEntryPoint pAddEP, USHORT UnitHandle,
                        USHORT FAR *NumHeads, USHORT FAR *TrackSectors);
USHORT InitReadSector (PADDEntryPoint pAddEP, USHORT UnitHandle,
                       ULONG SectorRBA, ULONG ppSectorBuf);
USHORT InitSendIORB(PIORB pIORB, PADDEntryPoint pADDEntry);

#endif
