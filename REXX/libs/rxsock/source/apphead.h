/*
 *  RxSock
 *  Copyright (C) 2003-2003  Mark Hessling <M.Hessling@qut.edu.au>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define RXPACKAGE_MAGIC_NUMBER    1028948572
#define RXPACKAGE_DEBUG_VAR       "RXSOCK_DEBUG"

#define RXPACKAGENAME             "rxsock"
#define GETPACKAGEINITIALISER()   getRxSockInitialiser()
#define GETPACKAGETERMINATOR()    getRxSockTerminator()
#define GETPACKAGEFUNCTIONS()     getRxSockFunctions()
#define GETPACKAGECONSTANTS()     getRxSockConstants()
#define GETPACKAGESUBCOMHANDLER() getRxSockSubcomHandler()
#define GETPACKAGEINITHANDLER()   getRxSockInitHandler()
#define PACKAGEUSAGE()            RxSockUsage()

