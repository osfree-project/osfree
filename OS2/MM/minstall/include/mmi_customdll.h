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

// Before executing Custom-DLLs, MINSTALL_LinkInImports() must have been called

#ifndef _MMI_CUSTOMDLL_H_
#define _MMI_CUSTOMDLL_H_

BOOL MINSTALL_CreateCustomAPIThread (void);
VOID MINSTALL_RemoveCustomAPIThread (void);
BOOL MINSTALL_ExecuteCustomDLLs (void);
BOOL MINSTALL_ExecuteCustomTermDLLs (void);

// Now the whole MINSTALL CustomDLL-API rewritten
#define MINSTOLD_MAXSETUPSTRING    100
#define MINSTOLD_MAXFOLDERNAME     150
#define MINSTOLD_MAXRESPLENGTH     256

#define MINSTOLD_MAXWPCLASSNAME    256
#define MINSTOLD_MAXWPTITLE        256
#define MINSTOLD_MAXWPSETUPSTRING 1024
#define MINSTOLD_MAXWPLOCATION     256

#define MINSTOLD_MAXNAME          1024

// Public variables...
PMINSTINIHEADER CustomAPI_INIChange1stEntryPtr;
PMINSTINIHEADER CustomAPI_INIChangeLastEntryPtr;

#pragma pack(1)

/***************************************************************************/
/* This is a function pointer prototype that describes the entry point     */
/* into the device specific DLL.                                           */
/*                                                                         */
/* Parameters:  HWND - Owner handle.                                       */
/*              PSZ  - Source path.                                        */
/*              PSZ  - Destination Drive (drive letter, colon - eg.  "X:") */
/*              PSZ  - Dll input parms in script file (DLL specific)       */
/*              HWND - Object window that receives message to do MCI and   */
/*                     CONFIG.SYS work.                                    */
/*              PSZ  - PSZ used for response file. (Is a CHAR [256] and    */
/*                     used for IN/OUT)                                    */
/***************************************************************************/
typedef ULONG APIENTRY CUSTOMDLL_ENTRYFUNC (HWND OwnerHandle, PSZ SourcePath, PSZ DestDrive, PSZ DllInputParms, HWND ObjectHandle, PSZ ResponseFile);
typedef CUSTOMDLL_ENTRYFUNC *PCUSTOMDLL_ENTRYFUNC;

/***************************************************************************/
/* IM_EA_JOIN                                                              */
/*                                                                         */
/* mp1 = 0;                ** Not used                                     */
/* mp2 = MPFROMP(PINSTEAJOIN);                                             */
/*                                                                         */
/* Return EA join result.                                                  */
/***************************************************************************/
#define MINSTOLD_EA_JOIN_MSGID              0x057D

typedef struct _MINSTOLD_EA_JOIN {
   CHAR achFileName[CCHMAXPATH];
   CHAR achEAFileName[CCHMAXPATH];
 } MINSTOLD_EA_JOIN;
typedef MINSTOLD_EA_JOIN *PMINSTOLD_EA_JOIN;

/***************************************************************************/
/* IM_EA_LONG_NAME_JOIN                                                    */
/*                                                                         */
/* mp1 = 0;                ** Not used                                     */
/* mp2 = MPFROMP(PINSTEALONGNAMEJOIN);                                     */
/*                                                                         */
/* Return EA long name creation result.                                    */
/***************************************************************************/
#define MINSTOLD_EA_LONGNAMEJOIN_MSGID      0x0581

typedef struct _MINSTOLD_EA_LONGNAMEJOIN {
   CHAR achLongName[CCHMAXPATH];
   CHAR achLongFileName[CCHMAXPATH];
   CHAR achEALongFileName[CCHMAXPATH];
 } MINSTOLD_EA_LONGNAMEJOIN;
typedef MINSTOLD_EA_LONGNAMEJOIN *PMINSTOLD_EA_LONGNAMEJOIN;

/***************************************************************************/
/* IM_MCI_EXTENDED_SYSINFO - Maps to MCI_SYSINFO.                          */
/*                                                                         */
/* mp1 = msg;                                                              */
/* mp2 = pvoid;                                                            */
/*                                                                         */
/* This will map to:                                                       */
/*         sysinfo.dwItem       = (ULONG)LONGFROMMP(mp1);                  */
/*         sysinfo.pSysInfoParm = PVOIDFROMMP(mp2);                        */
/*         mciSendCommand((USHORT)0,                                       */
/*                        (USHORT)MCI_SYSINFO,                             */
/*                        (ULONG)MCI_SYSINFO_ITEM,                         */
/*                        (ULONG)&sysinfo,                                 */
/*                        (USHORT)0);                                      */
/***************************************************************************/
#define MINSTOLD_MCI_SYSINFO_MSGID 0x057E

/***************************************************************************/
/* IM_MMIO_INSTALL - Install an IO-Proc                                    */
/*                                                                         */
/* mp1 = 0;                        ** Not used                             */
/* mp2 = (PINSTIOPROC)&instioproc; ** pointer to the INSTIOPROC struct     */
/***************************************************************************/
#define MINSTOLD_MMIO_INSTALL_MSGID         0x057F

typedef struct _MINSTOLD_MMIO_INSTALL {
   CHAR  fccIOProc[5];
   CHAR  szDLLName[CCHMAXPATH];
   CHAR  szProcName[32];
   ULONG ulFlags;
   ULONG ulExtendLen;
   ULONG ulMediaType;
   ULONG ulIOProcType;
   CHAR  szDefExt[4];
 } MINSTOLD_MMIO_INSTALL;
typedef MINSTOLD_MMIO_INSTALL *PMINSTOLD_MMIO_INSTALL;

/***************************************************************************/
/* IM_LOG_ERROR - Write a message to the log file.                         */
/*                                                                         */
/* mp1 = (PSZ)pszStatement;   ** Should end with a '\n' and be NULL        */
/*                            **  terminated                               */
/* mp2 = 0;                   ** Unused                                    */
/***************************************************************************/
#define MINSTOLD_LOG_ERROR_MSGID            0x0573

/***************************************************************************/
/* CONFIGDATA - Pointer to this structure is passed in mp1 in all CONFIG   */
/*              messages.                                                  */
/***************************************************************************/
typedef struct _MINSTOLD_CONFIGDATA {
   LONG lLine;
   LONG lBufferLen;
   PSZ  pszBuffer;
   BOOL fAdd;
 } MINSTOLD_CONFIGDATA;
typedef MINSTOLD_CONFIGDATA *PMINSTOLD_CONFIGDATA;

/* Line positions */
#define MINSTOLD_CONFIG_TOP      -1
#define MINSTOLD_CONFIG_BOTTOM   -2
#define MINSTOLD_CONFIG_NEXT     -3
#define MINSTOLD_CONFIG_PREV     -4
#define MINSTOLD_CONFIG_CURRENT  -5

/***************************************************************************/
/* IM_CONFIGENUMERATE - Get a line from the config.sys file.               */
/*                                                                         */
/* mp1 = (PCONFIGDATA)pconfdata;                                           */
/* mp2 = 0                                                                 */
/***************************************************************************/
#define MINSTOLD_CONFIG_ENUMERATE_MSGID     0x0574

/***************************************************************************/
/* IM_CONFIGUPDATE - Update an entry in the config.sys file.               */
/*                                                                         */
/* mp1 = (PCONFIGDATA)pconfdata;                                           */
/* mp2 = 0                                                                 */
/***************************************************************************/
#define MINSTOLD_CONFIG_UPDATE_MSGID        0x0575

/***************************************************************************/
/* IM_CONFIGMERGE - Merge data into an existing config.sys entry.          */
/*                                                                         */
/* mp1 = (PCONFIGDATA)pconfdata;                                           */
/* mp2 = 0                                                                 */
/***************************************************************************/
#define MINSTOLD_CONFIG_MERGE_MSGID         0x0576

/***************************************************************************/
/* IM_CONFIGEREPLACE - Replace an entry that exists in config.sys.         */
/*                                                                         */
/* mp1 = (PCONFIGDATA)pconfdata;                                           */
/* mp2 = 0                                                                 */
/***************************************************************************/
#define MINSTOLD_CONFIG_REPLACE_MSGID       0x0577
// replaces CONFIG.SYS *FILE* - bad bad bad boy

/***************************************************************************/
/* IM_CONFIGNEW - Add a line to the config.sys file.                       */
/*                                                                         */
/* mp1 = (PCONFIGDATA)pconfdata;                                           */
/* mp2 = 0                                                                 */
/***************************************************************************/
#define MINSTOLD_CONFIG_NEW_MSGID           0x0578

/***************************************************************************/
/* IM_CONFIGDELETE - Delete a line (REM it out) from the config.sys file.  */
/*                                                                         */
/* mp1 = (PCONFIGDATA)pconfdata;                                           */
/* mp2 = 0                                                                 */
/***************************************************************************/
#define MINSTOLD_CONFIG_DELETE_MSGID        0x0579

/***************************************************************************/
/* IM_CONFIGQUERYCHANGED - Returns TRUE if config.sys has changed else     */
/*                         FALSE.                                          */
/*                                                                         */
/* mp1 = 0                                                                 */
/* mp2 = 0                                                                 */
/***************************************************************************/
#define MINSTOLD_CONFIG_QUERYCHANGED_MSGID  0x057A

/***************************************************************************/
/* IM_MIDIMAP_INSTALL - Install a midi map.                                */
/*                                                                         */
/* mp1 = 0;                ** Not used                                     */
/* mp2 = MPFROMP(pmidimapinstalldata);                                     */
/*                                                                         */
/* Return 0 on success, else error occurred.                               */
/***************************************************************************/
#define MINSTOLD_MIDIMAP_INSTALL_MSGID      0x057B

typedef struct _MINSTOLD_MIDIMAP_INSTALL {
   CHAR  achIniName[CCHMAXPATH];
   CHAR  achAppName[1024];
   CHAR  achKeyName[1024];
   CHAR  achDllName[CCHMAXPATH];
   ULONG ulResourceID;
 } MINSTOLD_MIDIMAP_INSTALL;
typedef MINSTOLD_MIDIMAP_INSTALL *PMINSTOLD_MIDIMAP_INSTALL;

/***************************************************************************/
/* IM_MCI_SEND_COMMAND - Send an MCI command                               */
/*                                                                         */
/* mp1 = 0;                ** Not used                                     */
/* mp2 = MPFROMP(PINSTMCISENDCOMMAND);                                     */
/*                                                                         */
/* Return mciSendCommand result.                                           */
/***************************************************************************/
#define MINSTOLD_MCI_SENDCOMMAND_MSGID      0x057C

typedef struct _MINSTOLD_MCI_SENDCOMMAND {
   USHORT wDeviceID;
   USHORT wMessage;
   ULONG  dwParam1;
   PVOID  dwParam2;
   USHORT wUserParm;
 } MINSTOLD_MCI_SENDCOMMAND;
typedef MINSTOLD_MCI_SENDCOMMAND *PMINSTOLD_MCI_SENDCOMMAND;

// ******************* ???
// mp1 = PMFROMP(PSZ) - Name of the file needed
// mp2 = PMFROMP(PSZ) - The full path to the file (CCHMAXPATH)
#define MINSTOLD_QUERYPATH_MSGID            0x0582

/***************************************************************************/
/* IM_CODECINSTALL - Install a Codec-Proc                                  */
/*                                                                         */
/* mp1 = 0;                        ** Not used                             */
/* mp2 = (PINSTCODECINIFILEINFO)&instioproc;                               */
/*                                 ** pointer to the INSTIOPROC struct     */
/*                                                            feature 5572 */
/***************************************************************************/
#define MINSTOLD_MMIO_CODEC1INSTALL_MSGID   0x0583
#define MINSTOLD_MMIO_CODEC2INSTALL_MSGID   0x0584
#define MINSTOLD_MMIO_CODECDELETE_MSGID     0x0585

typedef struct _MINSTOLD_MMIO_CODEC {
   ULONG       ulStructLen;
   CHAR        fcc[5];
   CHAR        szDLLName[CCHMAXPATH];
   CHAR        szProcName[32];
   union {
      ULONG  ulCodecCompType;
      CHAR   fccCodecCompType[5];
    } x;
   ULONG       ulCompressSubType;
   ULONG       ulMediaType;
   ULONG       ulCapsFlags;
   ULONG       ulFlags;
   CHAR        szHWID[32];
   ULONG       ulMaxSrcBufLen;
   ULONG       ulSyncMethod;
   ULONG       fccPreferredFormat;
   ULONG       ulXalignment;
   ULONG       ulYalignment;
   CHAR        szSpecInfo[32];
 } MINSTOLD_MMIO_CODEC;
typedef MINSTOLD_MMIO_CODEC *PMINSTOLD_MMIO_CODEC;

/***************************************************************************/
/* IM_PROFILESTRINGINSTALL - add a profile string to an INI file           */
/*                                                                         */
/* mp1 = 0;                     ** Not used                                */
/* mp2 = MPFROMP(PPROFILESTRINGDATA)                                       */
/*                              ** pointer to the PROFILESTRING struct     */
/*                                                           feature 12057 */
/***************************************************************************/
#define MINSTOLD_PRF_STRINGDATA_MSGID       0x0586

typedef struct _MINSTOLD_PRF_STRINGDATA {
   CHAR  achInisName[CCHMAXPATH];
   CHAR  achAppsName[MINSTOLD_MAXNAME];
   CHAR  achKeysName[MINSTOLD_MAXNAME];
   CHAR  achDatasName[MINSTOLD_MAXNAME];
 } MINSTOLD_PRF_STRINGDATA;
typedef MINSTOLD_PRF_STRINGDATA *PMINSTOLD_PRF_STRINGDATA;

/***************************************************************************/
/* IM_APPEND_PRF_STRING - ensure that a sub-string resides within a key    */
/*                        of an INI file, append if not present            */
/*                                                                         */
/* mp1 = 0;                        ** Not used                             */
/* mp2 = MPFROMP(PPRFAPPENDDATA);                                          */
/*                                 ** pointer to the PRFAPPENDDATA struct  */
/***************************************************************************/
#define MINSTOLD_PRF_APPENDDATA_MSGID       0x0588

typedef struct _MINSTOLD_PRF_APPENDDATA {
   CHAR    achIniFile[CCHMAXPATH];
   CHAR    achAppName[MINSTOLD_MAXNAME];
   CHAR    achKeyName[MINSTOLD_MAXNAME];
   CHAR    achDefault[MINSTOLD_MAXNAME];
 } MINSTOLD_PRF_APPENDDATA;
typedef MINSTOLD_PRF_APPENDDATA *PMINSTOLD_PRF_APPENDDATA;

/***************************************************************************/
/* IM_SPI_INSTALL - Install stream protocol information                    */
/*                                                                         */
/* mp1 = 0;                ** Not used                                     */
/* mp2 = (PSZ)pszDllPath;  ** Fully qualified path of a SPI resource DLL   */
/***************************************************************************/
#define MINSTOLD_SPI_INSTALL_MSGID          0x0580

/***************************************************************************/
/* IM_CREATE_WPS_OBJECT - Install a folder and its contents                */
/*                                                                         */
/* mp1 = 0;                ** Not used                                     */
/* mp2 = MPFROMP(pinstOBJECTdata);                                         */
/***************************************************************************/
#define MINSTOLD_WPS_CREATEOBJECT_MSGID     0x0571

typedef struct _MINSTOLD_WPS_CREATEOBJECT {
   CHAR  achClassName[MINSTOLD_MAXWPCLASSNAME];
   CHAR  achTitle[MINSTOLD_MAXWPTITLE];
   CHAR  achSetupString[MINSTOLD_MAXWPSETUPSTRING];
   CHAR  achLocation[MINSTOLD_MAXWPLOCATION];
   ULONG ulFlags;
 } MINSTOLD_WPS_CREATEOBJECT;
typedef MINSTOLD_WPS_CREATEOBJECT *PMINSTOLD_WPS_CREATEOBJECT;

/***************************************************************************/
/* IM_DESTROY_WPS_OBJECT - Install a folder and its contents               */
/*                                                                         */
/* mp1 = 0;                ** Not used                                     */
/* mp2 = MPFROMP(HOBJECT); ** or "<OBJECT_ID>"                             */
/***************************************************************************/
#define MINSTOLD_WPS_DESTROYOBJECT_MSGID    0x0572

#define MINSTOLD_WPS_WPCLASS_MSGID          0x0587
typedef struct _MINSTOLD_WPS_WPCLASS {
   CHAR  achClassNewName[MINSTOLD_MAXNAME];
   CHAR  achDllName[CCHMAXPATH];
   CHAR  achReplaceClass[MINSTOLD_MAXNAME];
 } MINSTOLD_WPS_WPCLASS;
typedef MINSTOLD_WPS_WPCLASS *PMINSTOLD_WPS_WPCLASS;

#define MINSTOLD_RETNOERROR               0
#define MINSTOLD_RETERRBASE               0xFF00
#define MINSTOLD_RETERRSTART              (MINSTOLD_RETERRBASE+1)

#define MINSTOLD_RETOPENFAILED            (MINSTOLD_RETERRSTART+  1)
#define MINSTOLD_RETOUTOFMEMORY           (MINSTOLD_RETERRSTART+  2)
#define MINSTOLD_RETINVALIDPARAMETER      (MINSTOLD_RETERRSTART+  3)
#define MINSTOLD_RETINVALIDNUMFLAG        (MINSTOLD_RETERRSTART+  4)
#define MINSTOLD_RETATTOP                 (MINSTOLD_RETERRSTART+  5)
#define MINSTOLD_RETATBOTTOM              (MINSTOLD_RETERRSTART+  6)
#define MINSTOLD_RETBUFFEROVERFLOW        (MINSTOLD_RETERRSTART+  7)
#define MINSTOLD_RETLINENOTFOUND          (MINSTOLD_RETERRSTART+  8)
#define MINSTOLD_RETCANTPARSEBUFFER       (MINSTOLD_RETERRSTART+  9)
#define MINSTOLD_RETVARIABLENOTFOUND      (MINSTOLD_RETERRSTART+ 10)

#pragma pack()

#endif
