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

#ifndef _E2VIRTIO_H_
#define _E2VIRTIO_H_

#include "os2head.h"

USHORT DoVirtualIO (USHORT Modifier, NPVirtUnitRec pUnitRec, ULONG SectorRBA,
                    PSCATGATENTRY pSGList, USHORT cSGList,ULONG XferOffset);

USHORT ReadFakeSector (NPVirtUnitRec pUnitRec, ULONG SectorRBA,
                       PSCATGATENTRY pSGList, USHORT cSGList,
                       ULONG XferOffset);
USHORT WriteFakeSector (NPVirtUnitRec pUnitRec, ULONG SectorRBA,
                       PSCATGATENTRY pSGList, USHORT cSGList,
                       ULONG XferOffset);

VOID ComputeCHS (ULONG RBA, USHORT GeoNumHead, USHORT GeoTrackSec,
                 USHORT FAR *CylSec, UCHAR FAR *Head);
USHORT CopyToSGList (void FAR *Buffer, ULONG Count, PSCATGATENTRY pSGList,
                     USHORT cSGList, ULONG XferOffset);

#endif
