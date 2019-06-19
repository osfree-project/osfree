//
// MINSTALL.DLL (c) Copyright 2002-2005 Martin Kiewitz
//
// This file is part of MINSTALL.DLL for OS/2 / eComStation
//
// MINSTALL.DLL is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
// MINSTALL.DLL is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
//  along with MINSTALL.DLL.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _MMI_BASESCR_H_
#define _MMI_BASESCR_H_

BOOL MINSTALL_LoadMasterControl (void);
BOOL MINSTALL_LoadFileControl (void);
BOOL MINSTALL_FilterControlFileData (void);
BOOL MINSTALL_GeneratePublicGroup (void);
BOOL MINSTALL_SaveInstalledVersions (void);
VOID MINSTALL_CleanUpMasterControl (void);
VOID MINSTALL_CleanUpFileControl (void);
VOID MINSTALL_CleanUpPublicGroup (void);

#endif
