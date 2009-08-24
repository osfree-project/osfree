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

#ifndef _MMI_INISTUFF_H_
#define _MMI_INISTUFF_H_


// Byte-Packing, these structures are fixed. They must not get optimized
#pragma pack(1)

typedef struct _MINSTINIHEADER {
   ULONG      ID;
   ULONG      Size;
   PVOID      NextPtr;
 } MINSTINIHEADER;
typedef MINSTINIHEADER *PMINSTINIHEADER;

// ****************************************************************************

// Some of those structures exist in MM-OS/2 headers, but I redefine them here
//  cause some of them spread havoc on VAC3. Some of them are also pretty
//  undocumented or are used in multiple ways (mmio-(Codec1/Codec2)-Install)

typedef struct _EA_JOINEA {
   MINSTINIHEADER Header;
   CHAR           JoinFileName[MINSTMAX_PATHLENGTH];
   CHAR           JoinEAFileName[MINSTMAX_PATHLENGTH];
 } EA_JOINEA;
typedef EA_JOINEA *PEA_JOINEA;
#define EA_JOINEA_ID                        0xD56F3A0A

typedef struct _EA_JOINLONGNAMEEA {
   MINSTINIHEADER Header;
   CHAR           JoinLongName[MINSTMAX_PATHLENGTH];
   CHAR           JoinLongFileName[MINSTMAX_PATHLENGTH];
   CHAR           JoinEALongFileName[MINSTMAX_PATHLENGTH];
 } EA_JOINLONGNAMEEA;
typedef EA_JOINLONGNAMEEA *PEA_JOINLONGNAMEEA;
#define EA_JOINLONGNAMEEA_ID                0x0A110003

#define MCIMAX_DEVICENAMELENGTH    20
#define MCIMAX_VERSIONLENGTH        6
#define MCIMAX_PRODLENGTH          40
#define MCIMAX_PDDNAMELENGTH        9
#define MCIMAX_ALIASNAMELENGTH     20
#define MCIMAX_DEVPARAMSLENGTH    128
#define MCIMAX_CLASSES             10    // Actual Limit 9 Classes
#define MCIMAX_CONNECTORS          10
#define MCIMAX_EXTENSIONS          25
#define MCIMAX_EXTENSIONNAMELENGTH  4
#define MCIMAX_TYPELISTLENGTH     257

typedef struct _MCI_MCIINSTALLDRV {
   MINSTINIHEADER Header;
   CHAR           szInstallName[MCIMAX_DEVICENAMELENGTH];
   USHORT         usDeviceType;
   ULONG          ulDeviceFlag;
   CHAR           szVersionNumber[MCIMAX_VERSIONLENGTH];
   CHAR           szProductInfo[MCIMAX_PRODLENGTH];
   CHAR           szMCDDriver[MCIMAX_DEVICENAMELENGTH];
   CHAR           szVSDDriver[MCIMAX_DEVICENAMELENGTH];
   CHAR           szPDDName[MCIMAX_PDDNAMELENGTH];
   CHAR           szMCDTable[MCIMAX_DEVICENAMELENGTH];
   CHAR           szVSDTable[MCIMAX_DEVICENAMELENGTH];
   USHORT         usShareType;
   CHAR           szResourceName[MCIMAX_DEVICENAMELENGTH];
   USHORT         usResourceUnits;
   USHORT         usResourceClasses;
   USHORT         ausClassArray[MCIMAX_CLASSES];
   USHORT         ausValidClassArray[MCIMAX_CLASSES][MCIMAX_CLASSES];
 } MCI_MCIINSTALLDRV;
typedef MCI_MCIINSTALLDRV *PMCI_MCIINSTALLDRV;
#define MCI_MCIINSTALLDRV_ID                0x92325D4C

typedef struct _MCI_MCIINSTALLCONNARRAY {
   USHORT         usConnectType;
   CHAR           szToInstallName[MCIMAX_DEVICENAMELENGTH];
   USHORT         usToConnectIndex;
 } MCI_MCIINSTALLCONNARRAY;
typedef MCI_MCIINSTALLCONNARRAY *PMCI_MCIINSTALLCONNARRAY;

typedef struct _MCI_MCIINSTALLCONN {
   MINSTINIHEADER Header;
   CHAR           szInstallName[MCIMAX_DEVICENAMELENGTH];
   USHORT         usNumConnectors;
   MCI_MCIINSTALLCONNARRAY ConnectorList[MCIMAX_CONNECTORS];
 } MCI_MCIINSTALLCONN;
typedef MCI_MCIINSTALLCONN *PMCI_MCIINSTALLCONN;
#define MCI_MCIINSTALLCONN_ID               0x37AC2C3F

typedef struct _MCI_MCIINSTALLPARM {
   MINSTINIHEADER Header;
   CHAR           szInstallName[MCIMAX_DEVICENAMELENGTH];
   CHAR           szDevParams[MCIMAX_DEVPARAMSLENGTH];
 } MCI_MCIINSTALLPARM;
typedef MCI_MCIINSTALLPARM *PMCI_MCIINSTALLPARM;
#define MCI_MCIINSTALLPARM_ID               0x00E0F5A3

typedef struct _MCI_MCIINSTALLALIAS {
   MINSTINIHEADER Header;
   CHAR           szInstallName[MCIMAX_DEVICENAMELENGTH];
   CHAR           szAliasName[MCIMAX_ALIASNAMELENGTH];
 } MCI_MCIINSTALLALIAS;
typedef MCI_MCIINSTALLALIAS *PMCI_MCIINSTALLALIAS;
#define MCI_MCIINSTALLALIAS_ID              0x7C939816

typedef struct _MCI_MCIINSTALLEXT {
   MINSTINIHEADER Header;
   CHAR           szInstallName[MCIMAX_DEVICENAMELENGTH];
   USHORT         usNumExtensions;
   CHAR           szExtension[MCIMAX_EXTENSIONS][MCIMAX_EXTENSIONNAMELENGTH];
 } MCI_MCIINSTALLEXT;
typedef MCI_MCIINSTALLEXT *PMCI_MCIINSTALLEXT;
#define MCI_MCIINSTALLEXT_ID                0x8711BEDD

typedef struct _MCI_MCIINSTALLTYPES {
   MINSTINIHEADER Header;
   CHAR           szInstallName[MCIMAX_DEVICENAMELENGTH];
   CHAR           szTypes[MCIMAX_TYPELISTLENGTH];
 } MCI_MCIINSTALLTYPES;
typedef MCI_MCIINSTALLTYPES *PMCI_MCIINSTALLTYPES;
#define MCI_MCIINSTALLTYPES_ID              0xC4CF7AB2

#define MMIOMAX_DLLLENGTH            CCHMAXPATH
#define MMIOMAX_PROCNAMELENGTH       32
#define MMIOMAX_EXTENSIONNAMELENGTH   4
#define MMIOMAX_HWNAMELENGTH         32
#define MMIOMAX_CODECINFOLENGTH       8
#define MMIOMAX_SPECINFOLENGTH       32

// MMIO_MMIOINSTALL -> MMINIFILEINFO
// MMIO_MMIOCODEC   -> CODECINIFILEINFO

typedef struct _MMIO_MMIOINSTALL {
   MINSTINIHEADER Header;
   ULONG          fccIOProc;
   CHAR           szDLLName[MMIOMAX_DLLLENGTH];
   CHAR           szProcName[MMIOMAX_PROCNAMELENGTH];
   ULONG          dwFlags;
   ULONG          dwExtendLen;
   ULONG          dwMediaType;
   ULONG          dwIOProcType;
   CHAR           szDefExt[MMIOMAX_EXTENSIONNAMELENGTH];
 } MMIO_MMIOINSTALL;
typedef MMIO_MMIOINSTALL *PMMIO_MMIOINSTALL;
#define MMIO_MMIOINSTALL_ID                 0x12709014

typedef struct _MMIO_MMIOCODEC {
   MINSTINIHEADER Header;
   ULONG          ulStructLen;
   ULONG          fcc;
   CHAR           szDLLName[MMIOMAX_DLLLENGTH];
   CHAR           szProcName[MMIOMAX_PROCNAMELENGTH];
   ULONG          ulCompressType;
   ULONG          ulCompressSubType;
   ULONG          ulMediaType;
   ULONG          ulCapsFlags;
   ULONG          ulFlags;
   CHAR           szHWID[MMIOMAX_HWNAMELENGTH];
   ULONG          ulMaxSrcBufLen;
   ULONG          ulSyncMethod;
   ULONG          fccPreferredFormat;
   ULONG          ulXalignment;
   ULONG          ulYalignment;
   CHAR           szSpecInfo[MMIOMAX_SPECINFOLENGTH];
   } MMIO_MMIOCODEC;
typedef MMIO_MMIOCODEC *PMMIO_MMIOCODEC;
#define MMIO_MMIOCODECDELETE_ID             0xA359F6F9
#define MMIO_MMIOCODEC1INSTALL_ID           0x75AE2FBC
#define MMIO_MMIOCODEC2INSTALL_ID           0xFB21285F

typedef struct _PRF_PROFILEDATA {
   MINSTINIHEADER Header;
   CHAR           Ini[MINSTMAX_PATHLENGTH];
   CHAR           AppName[MINSTMAX_STRLENGTH];
   CHAR           KeyName[MINSTMAX_STRLENGTH];
   CHAR           Dll[MINSTMAX_PATHLENGTH];
   ULONG          Id;
 } PRF_PROFILEDATA;
typedef PRF_PROFILEDATA *PPRF_PROFILEDATA;
#define PRF_PROFILEDATA_ID                  0xC339D14E

typedef struct _PRF_PROFILESTRING {
   MINSTINIHEADER Header;
   CHAR           Inis[MINSTMAX_PATHLENGTH];
   CHAR           AppNames[MINSTMAX_STRLENGTH];
   CHAR           KeyNames[MINSTMAX_STRLENGTH];
   CHAR           Datas[MINSTMAX_PATHLENGTH];
 } PRF_PROFILESTRING;
typedef PRF_PROFILESTRING *PPRF_PROFILESTRING;
#define PRF_PROFILESTRING_ID                0xC51EC302

typedef LHANDLE HPROGRAM;
typedef HPROGRAM *PHPROGRAM;
typedef LHANDLE HAPP;

typedef LHANDLE HINI;
typedef HINI *PHINI;

HINI APIENTRY PrfOpenProfile(HAB hab, PSZ pszFileName);
BOOL APIENTRY PrfCloseProfile(HINI hini);
BOOL APIENTRY PrfWriteProfileString(HINI hini, PSZ pszApp, PSZ pszKey, PSZ pszData);
BOOL APIENTRY PrfWriteProfileData(HINI hini, PSZ pszApp, PSZ pszKey, PVOID pData, ULONG cchDataLen);

typedef struct _SPI_SPIINSTALL {
   MINSTINIHEADER Header;
   CHAR           SpiDllName[MINSTMAX_PATHLENGTH];
 } SPI_SPIINSTALL;
typedef SPI_SPIINSTALL *PSPI_SPIINSTALL;
#define SPI_SPIINSTALL_ID                   0x78CA5190

typedef struct _WPS_CREATEOBJECT {          // Originally named WPS_WPOBJECT
   MINSTINIHEADER Header;
   CHAR           WPClassName[MINSTMAX_STRLENGTH];
   CHAR           WPTitle[MINSTMAX_STRLENGTH];
   CHAR           WPSetupString[MINSTMAX_PATHLENGTH]; // No path but huge buffer
   CHAR           WPLocation[MINSTMAX_STRLENGTH];
   ULONG          WPFlags;
 } WPS_CREATEOBJECT;
typedef WPS_CREATEOBJECT *PWPS_CREATEOBJECT;
#define WPS_CREATEOBJECT_ID                 0xF08301C8

typedef struct _WPS_DESTROYOBJECT {
   MINSTINIHEADER Header;
   CHAR           WPDestroyObjectID[MINSTMAX_STRLENGTH];
 } WPS_DESTROYOBJECT;
typedef WPS_DESTROYOBJECT *PWPS_DESTROYOBJECT;
#define WPS_DESTROYOBJECT_ID                0x92D45D84

typedef struct _WPS_WPCLASS {
   MINSTINIHEADER Header;
   CHAR           WPClassNameNew[MINSTMAX_STRLENGTH];
   CHAR           WPDllName[MINSTMAX_PATHLENGTH];
   CHAR           WPReplaceClass[MINSTMAX_STRLENGTH];
 } WPS_WPCLASS;
typedef WPS_WPCLASS *PWPS_WPCLASS;
#define WPS_WPCLASS_ID                      0xABAD2C82

// Public variables...
MINSTINI_DEFENTRY MINSTINI_FuncList[];
MINSTINI_DEFENTRY MINSTINIWPS_WPObject[];

#pragma pack()

#endif
