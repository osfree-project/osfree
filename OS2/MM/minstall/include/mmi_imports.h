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

// Contains all imports used by minstall. We have to dynamically link
//  because otherwise FILT.DLL f00ks up, if no MMOS/2 installed.
//  These routines will only get loaded during MINSTALL_InstallPackage()

#ifndef _MMI_IMPORTS_H_
#define _MMI_IMPORTS_H_

// Internal, export from SSMINI.DLL
APIRET SpiInstall (PSZ SpiDllName);
typedef APIRET (*CODE_SPIINSTALL) (PSZ SpiDllName);

// Internal, import from MDM.DLL
ULONG APIENTRY mciSendCommand (USHORT usDeviceID, USHORT usMessage,
                               ULONG ulParam1, PVOID pParam2, USHORT usUserParm);
typedef ULONG (APIENTRY *CODE_MCISENDCOMMAND) (USHORT usDeviceID, USHORT usMessage,
                               ULONG ulParam1, PVOID pParam2, USHORT usUserParm);
typedef ULONG (APIENTRY *CODE_MCISETSYSVALUE) (USHORT iSysValue, USHORT pValue);

// Internal, import from MMIO.DLL
ULONG APIENTRY mmioMigrateIniFile (ULONG ulFlags);
// defined in mmos2.h ULONG APIENTRY mmioIniFileHandler (PMMIO_MMIOINSTALL IOProcInfo, ULONG dwFlags);
ULONG APIENTRY mmioIniFileCODEC (PMMIO_MMIOCODEC IOCodecInfo, ULONG ulFlags);
typedef ULONG (APIENTRY *CODE_MMIOMIGRATEINIFILE) (ULONG ulFlags);
typedef ULONG (APIENTRY *CODE_MMIOINIFILEHANDLER) (PMMIO_MMIOINSTALL IOProcInfo, ULONG dwFlags);
typedef ULONG (APIENTRY *CODE_MMIOINIFILECODEC) (PMMIO_MMIOCODEC IOCodecInfo, ULONG ulFlags);

#define MMIO_SUCCESS                     0

#define MMIO_INSTALLPROC        0x00000001
#define MMIO_REMOVEPROC         0x00000002
#define MMIO_FINDPROC           0x00000004
#define MMIO_MATCHFOURCC        0x00000040
#define MMIO_MATCHDLL           0x00000080
#define MMIO_MATCHPROCEDURENAME 0x00000100
#define MMIO_MATCHCOMPRESSTYPE  0x00000800
#define MMIO_FULLPATH           0x00000200
#define MMIO_EXTENDED_STRUCT    0x00001000

// Internal routines to Link-In Import functions
BOOL MINSTALL_LinkInImports (void);
VOID MINSTALL_CleanUpImports (void);
VOID MINSTALL_MigrateMMPMMMIOFile (void);
VOID MINSTALL_LowerMasterVolume (void);

// Linked in Code-pointers
CODE_MCISETSYSVALUE     CODE_MciSetSysValue;
CODE_SPIINSTALL         CODE_SpiInstallFunc;
CODE_MMIOINIFILEHANDLER CODE_mmioIniFileHandlerFunc;
CODE_MMIOINIFILECODEC   CODE_mmioIniFileCODECFunc;
CODE_MMIOMIGRATEINIFILE CODE_mmioMigrateIniFileFunc;

#endif
