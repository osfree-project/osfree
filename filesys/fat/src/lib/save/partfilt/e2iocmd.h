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

#ifndef _E2IOCMD_H_
#define _E2IOCMD_H_

#include "os2head.h"

void BadCommand (NPVirtUnitRec pUnitRec, PIORB pIORB);
void PartGetDeviceTable (NPVirtUnitRec pUnitRec, PIORB pIORB);
void PartChangeUnitInfo (NPVirtUnitRec pUnitRec, PIORB pIORB);
void PartGetGeometry (NPVirtUnitRec pUnitRec, PIORB pIORB);
void PartDoIO (NPVirtUnitRec pUnitRec, PIORB pIORB);
void PartGetUnitStatus (NPVirtUnitRec pUnitRec, PIORB pIORB);

#endif
