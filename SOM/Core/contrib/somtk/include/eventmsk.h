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


#ifndef EVENTMSK_H
#define EVENTMSK_H
 
/*
 * long is enforced due to 64bit systems
 * SOM ABI does not use the 'int' type much
 */

#define EMTimerEvent                    54L 
#define EMSignalEvent                   55L 
#define EMSinkEvent                     56L 
#define EMWorkProcEvent                 57L 
#define EMClientEvent                   58L
#define EMMsgQEvent						59L

#define EMInputReadMask					(1L<<0)
#define EMInputWriteMask				(1L<<1)
#define EMInputExceptMask				(1L<<2)

#define EMProcessTimerEvent             (1L<<0)
#define EMProcessSinkEvent              (1L<<1)
#define EMProcessWorkProcEvent          (1L<<2)
#define EMProcessClientEvent            (1L<<3)
#define EMProcessAllEvents              (1L<<6)

#endif   /* EVENTMSK_H */
