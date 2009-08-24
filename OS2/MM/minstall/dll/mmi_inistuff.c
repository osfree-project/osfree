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

#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#define INCL_OS2MM
#define INCL_MMIO_CODEC
#define INCL_AUDIO_CODEC_ONLY
#include <os2.h>
#include <pmdef.h>
#include <os2me.h>

#include <malloc.h>
#include <string.h>

#include <global.h>
#include <crcs.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mmi_public.h>
#include <mmi_types.h>
#include <mmi_main.h>
#include <mmi_inistuff.h>


MINSTINI_DEFENTRY MINSTINIEA_JoinEA[] = {
   { 0x0FD42A68,  TRUE, "JoinFileName",         NULL,                            0 },
   { 0xDF5CDE5B,  TRUE, "JoinEAFileName",       NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIEA_JoinLongNameEA[] = {
   { 0xCACCF1F1,  TRUE, "JoinLongName",         NULL,                            0 },
   { 0xD5C2CE0C,  TRUE, "JoinLongFileName",     NULL,                            0 },
   { 0xF01882D8,  TRUE, "JoinEALongFileName",   NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallDrvClassArray[] = {
   { 0x2FC58D01,  TRUE, "DrvClassNumber",       NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallDrv[] = {
   { 0xCAECA131,  TRUE, "DrvInstallName",       NULL,                            0 },
   { 0xF1558F48,  TRUE, "DrvDeviceType",        NULL,                            0 },
   { 0xAC7F33FB,  TRUE, "DrvDeviceFlag",        NULL,                            0 },
   { 0xDD6E470A,  TRUE, "DrvVersionNumber",     NULL,                            0 },
   { 0xB377234D,  TRUE, "DrvProductInfo",       NULL,                            0 },
   { 0xB4BFB2C2,  TRUE, "DrvMCDDriver",         NULL,                            0 },
   { 0xE6915F45, FALSE, "DrvVSDDriver",         NULL,                            0 },
   { 0xE8AB4FF7, FALSE, "DrvPDDName",           NULL,                            0 },
   { 0x2552FF41,  TRUE, "DrvMCDTable",          NULL,                            0 },
   { 0x635D4EF2, FALSE, "DrvVSDTable",          NULL,                            0 },
   { 0xC5E91968,  TRUE, "DrvShareType",         NULL,                            0 },
   { 0x25A89CAF,  TRUE, "DrvResourceName",      NULL,                            0 },
   { 0x7DAFC42F,  TRUE, "DrvResourceUnits",     NULL,                            0 },
   { 0x924AB2E1,  TRUE, "DrvClassArray",        MINSTINIMCI_MciInstallDrvClassArray, 10 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallConnArray[] = {
   { 0x39EDF742,  TRUE, "ConnType",             NULL,                            0 },
   { 0x77E86C5D,  TRUE, "ConnInstallTo",        NULL,                            0 },
   { 0xDEB82C0C,  TRUE, "ConnIndexTo",          NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallConn[] = {
   { 0x7C9375DB,  TRUE, "ConnInstallName",      NULL,                            0 },
   { 0x40B7450F,  TRUE, "ConnArray",            &MINSTINIMCI_MciInstallConnArray,10 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallParm[] = {
   { 0xBB3418BC,  TRUE, "ParmInstallName",      NULL,                            0 },
   { 0x7E2CCDF4,  TRUE, "ParmString",           NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallAlias[] = {
   { 0xDE60EB0E,  TRUE, "AliasInstallName",     NULL,                            0 },
   { 0xA278C064,  TRUE, "AliasString",          NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallExtArray[] = {
   { 0xFCD1A3BD,  TRUE, "ExtString",            NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallExt[] = {
   { 0x8187B8FE,  TRUE, "ExtInstallName",       NULL,                            0 },
   { 0x130025CB,  TRUE, "ExtArray",             MINSTINIMCI_MciInstallExtArray, MCIMAX_EXTENSIONS },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMCI_MciInstallTypes[] = {
   { 0x3DA680F8,  TRUE, "TypesInstallName",     NULL,                            0 },
   { 0x84C253AD,  TRUE, "TypesTypeList",        NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMMIO_mmioInstall[] = {
   { 0x874AF32B,  TRUE, "mmioFourCC",           NULL,                            0 },
   { 0x83BC6E92,  TRUE, "mmioDllName",          NULL,                            0 },
   { 0x383FA8EC,  TRUE, "mmioDllEntryPoint",    NULL,                            0 },
   { 0x6EA2DBC6,  TRUE, "mmioFlags",            NULL,                            0 },
   { 0x953B087F,  TRUE, "mmioExtendLen",        NULL,                            0 },
   { 0xB1824C61,  TRUE, "mmioMediaType",        NULL,                            0 },
   { 0xCD997BB5,  TRUE, "mmioIOProcType",       NULL,                            0 },
   { 0xF1124958,  TRUE, "mmioDefExt",           NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMMIO_mmioCodecDelete[] = {
   { 0xB8420316,  TRUE, "mmioDelLength",        NULL,                            0 },
   { 0x5CCDD4CA,  TRUE, "mmioDelFourCC",        NULL,                            0 },
   { 0x546A3B5B,  TRUE, "mmioDelDllName",       NULL,                            0 },
   { 0x75D262DB,  TRUE, "mmioDelDllEntryPoint", NULL,                            0 },
   { 0xB3D96C53,  TRUE, "mmioDelCompTypeFcc",   NULL,                            0 },
   { 0x6CCF6654,  TRUE, "mmioDelCompSubType",   NULL,                            0 },
   { 0xB0BB52CB,  TRUE, "mmioDelMediaType",     NULL,                            0 },
   { 0x524CECEC,  TRUE, "mmioDelFlags",         NULL,                            0 },
   { 0x09007EEC,  TRUE, "mmioDelCapsFlags",     NULL,                            0 },
   { 0xD8CF2CF5,  TRUE, "mmioDelHWName",        NULL,                            0 },
   { 0x2841E06C,  TRUE, "mmioDelMaxSrcBuf",     NULL,                            0 },
   { 0xD9643500,  TRUE, "mmioDelSyncMethod",    NULL,                            0 },
   { 0xF7CAA0ED,  TRUE, "mmioDelReserved1",     NULL,                            0 },
   { 0xFD9EE47D,  TRUE, "mmioDelXAlign",        NULL,                            0 },
   { 0x36C237D8,  TRUE, "mmioDelYAlign",        NULL,                            0 },
   { 0x2DCCCF4F,  TRUE, "mmioDelSpecInfo",      NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMMIO_mmioCodec1Install[] = {
   { 0xD1C05A78,  TRUE, "mmio1Length",          NULL,                            0 },
   { 0x354F8DA4,  TRUE, "mmio1FourCC",          NULL,                            0 },
   { 0xFE09F55D,  TRUE, "mmio1DllName",         NULL,                            0 },
   { 0x08D25727,  TRUE, "mmio1DllEntryPoint",   NULL,                            0 },
   { 0x487146E0,  TRUE, "mmio1CompTypeInt",     NULL,                            0 },
   { 0xAA46F050,  TRUE, "mmio1CompSubType",     NULL,                            0 },
   { 0x9A3DB099,  TRUE, "mmio1MediaType",       NULL,                            0 },
   { 0xD6D77C7F,  TRUE, "mmio1Flags",           NULL,                            0 },
   { 0x23869CBE,  TRUE, "mmio1CapsFlags",       NULL,                            0 },
   { 0xB14D759B,  TRUE, "mmio1HWName",          NULL,                            0 },
   { 0x02C7023E,  TRUE, "mmio1MaxSrcBuf",       NULL,                            0 },
   { 0x5C2B833A,  TRUE, "mmio1SyncMethod",      NULL,                            0 },
   { 0xDD4C42BF,  TRUE, "mmio1Reserved1",       NULL,                            0 },
   { 0x941CBD13,  TRUE, "mmio1XAlign",          NULL,                            0 },
   { 0x5F406EB6,  TRUE, "mmio1YAlign",          NULL,                            0 },
   { 0xC40509B4,  TRUE, "mmio1SpecInfo",        NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIMMIO_mmioCodec2Install[] = {
   { 0xE02840E5,  TRUE, "mmio2Length",          NULL,                            0 },
   { 0x04A79739,  TRUE, "mmio2FourCC",          NULL,                            0 },
   { 0x7086F2BE,  TRUE, "mmio2DllName",         NULL,                            0 },
   { 0x74B372FC,  TRUE, "mmio2DllEntryPoint",   NULL,                            0 },
   { 0x02CE28A7,  TRUE, "mmio2CompTypeFcc",     NULL,                            0 },
   { 0xDDD822A0,  TRUE, "mmio2CompSubType",     NULL,                            0 },
   { 0x710A0B9A,  TRUE, "mmio2MediaType",       NULL,                            0 },
   { 0x50430ED1,  TRUE, "mmio2Flags",           NULL,                            0 },
   { 0xC8B127BD,  TRUE, "mmio2CapsFlags",       NULL,                            0 },
   { 0x80A56F06,  TRUE, "mmio2HWName",          NULL,                            0 },
   { 0xE9F0B93D,  TRUE, "mmio2MaxSrcBuf",       NULL,                            0 },
   { 0xC5C9E53B,  TRUE, "mmio2SyncMethod",      NULL,                            0 },
   { 0x367BF9BC,  TRUE, "mmio2Reserved1",       NULL,                            0 },
   { 0xA5F4A78E,  TRUE, "mmio2XAlign",          NULL,                            0 },
   { 0x6EA8742B,  TRUE, "mmio2YAlign",          NULL,                            0 },
   { 0xFD883571,  TRUE, "mmio2SpecInfo",        NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIPRF_ProfileData[] = {
   { 0xE1F0F4E2,  TRUE, "ini",                  NULL,                            0 },
   { 0x027F2A21,  TRUE, "appname",              NULL,                            0 },
   { 0xF8048436,  TRUE, "keyname",              NULL,                            0 },
   { 0xAB74F1BC,  TRUE, "dll",                  NULL,                            0 },
   { 0x11D3633A,  TRUE, "id",                   NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIPRF_ProfileString[] = {
   { 0x6E859C63,  TRUE, "inis",                 NULL,                            0 },
   { 0x6C0B80B7,  TRUE, "appnames",             NULL,                            0 },
   { 0xEF227EDE,  TRUE, "keynames",             NULL,                            0 },
   { 0x384CCEAA,  TRUE, "datas",                NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINISPI_SpiInstall[] = {
   { 0xE906AF16,  TRUE, "SpiDllName",           NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIWPS_WPObject[] = {
   { 0x441CEA42,  TRUE, "WPClassName",          NULL,                            0 },
   { 0x6DD04D76,  TRUE, "WPTitle",              NULL,                            0 },
   { 0x10108A85,  TRUE, "WPSetupString",        NULL,                            0 },
   { 0x379F7EDF,  TRUE, "WPLocation",           NULL,                            0 },
   { 0x4DE374A7,  TRUE, "WPFlags",              NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIWPS_WPDestroyObject[] = {
   { 0x5BE681E8,  TRUE, "WPDestroyObjectID",    NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINIWPS_WPClass[] = {
   { 0x3ECA180C,  TRUE, "WPClassNameNew",       NULL,                            0 },
   { 0xB933A94C,  TRUE, "WPDllName",            NULL,                            0 },
   { 0x7F6632F4,  TRUE, "WPReplaceClass",       NULL,                            0 },
   { 0x00000000, FALSE, NULL,                   NULL,                            0 }
 };

MINSTINI_DEFENTRY MINSTINI_FuncList[] = {
   { EA_JOINEA_ID,              FALSE, "JoinEA",               MINSTINIEA_JoinEA,               sizeof(EA_JOINEA) },
   { EA_JOINLONGNAMEEA_ID,      FALSE, "JoinLongNameEA",       MINSTINIEA_JoinLongNameEA,       sizeof(EA_JOINLONGNAMEEA) },
   { MCI_MCIINSTALLDRV_ID,      FALSE, "MciInstallDrv",        MINSTINIMCI_MciInstallDrv,       sizeof(MCI_MCIINSTALLDRV) },
   { MCI_MCIINSTALLCONN_ID,     FALSE, "MciInstallConn",       MINSTINIMCI_MciInstallConn,      sizeof(MCI_MCIINSTALLCONN) },
   { MCI_MCIINSTALLPARM_ID,     FALSE, "MciInstallParm",       MINSTINIMCI_MciInstallParm,      sizeof(MCI_MCIINSTALLPARM) },
   { MCI_MCIINSTALLALIAS_ID,    FALSE, "MciInstallAlias",      MINSTINIMCI_MciInstallAlias,     sizeof(MCI_MCIINSTALLALIAS) },
   { MCI_MCIINSTALLEXT_ID,      FALSE, "MciInstallExt",        MINSTINIMCI_MciInstallExt,       sizeof(MCI_MCIINSTALLEXT) },
   { MCI_MCIINSTALLTYPES_ID,    FALSE, "MciInstallTypes",      MINSTINIMCI_MciInstallTypes,     sizeof(MCI_MCIINSTALLTYPES) },
   { MMIO_MMIOINSTALL_ID,       FALSE, "mmioInstall",          MINSTINIMMIO_mmioInstall,        sizeof(MMIO_MMIOINSTALL) },
   { MMIO_MMIOCODECDELETE_ID,   FALSE, "mmioCodecDelete",      MINSTINIMMIO_mmioCodecDelete,    sizeof(MMIO_MMIOCODEC) },
   { MMIO_MMIOCODEC1INSTALL_ID, FALSE, "mmioCodec1Install",    MINSTINIMMIO_mmioCodec1Install,  sizeof(MMIO_MMIOCODEC) },
   { MMIO_MMIOCODEC2INSTALL_ID, FALSE, "mmioCodec2Install",    MINSTINIMMIO_mmioCodec2Install,  sizeof(MMIO_MMIOCODEC) },
   { PRF_PROFILEDATA_ID,        FALSE, "ProfileData",          MINSTINIPRF_ProfileData,         sizeof(PRF_PROFILEDATA) },
   { PRF_PROFILESTRING_ID,      FALSE, "ProfileString",        MINSTINIPRF_ProfileString,       sizeof(PRF_PROFILESTRING) },
   { SPI_SPIINSTALL_ID,         FALSE, "SpiInstall",           MINSTINISPI_SpiInstall,          sizeof(SPI_SPIINSTALL) },
   { WPS_CREATEOBJECT_ID,       FALSE, "WPObject",             MINSTINIWPS_WPObject,            sizeof(WPS_CREATEOBJECT) },
   { WPS_DESTROYOBJECT_ID,      FALSE, "WPDestroyObject",      MINSTINIWPS_WPDestroyObject,     sizeof(WPS_DESTROYOBJECT) },
   { WPS_WPCLASS_ID,            FALSE, "WPClass",              MINSTINIWPS_WPClass,             sizeof(WPS_WPCLASS) },
   { 0x00000000,                FALSE, NULL,                   NULL }
 };
