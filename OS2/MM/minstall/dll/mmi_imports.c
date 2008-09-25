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
// #define INCL_OS2MM
#include <os2.h>
// #include <os2me.h>
#include <malloc.h>

#include <global.h>
#include <crcs.h>
#include <dll.h>
#include <file.h>
#include <globstr.h>
#include <msg.h>
#include <mciini.h>                         // My own MCI-SYSINFO routines...
#include <mmi_public.h>
#include <mmi_types.h>
#include <mmi_main.h>
#include <mmi_helper.h>
#include <mmi_msg.h>
#include <mmi_inistuff.h>
#include <mmi_imports.h>

HMODULE                 MINSTIMP_MDMHandle          = 0;
HMODULE                 MINSTIMP_SSMINIHandle       = 0;
HMODULE                 MINSTIMP_MMIOHandle         = 0;
CODE_MCISETSYSVALUE     CODE_MciSetSysValue         = 0;
CODE_SPIINSTALL         CODE_SpiInstallFunc         = 0;
CODE_MMIOINIFILEHANDLER CODE_mmioIniFileHandlerFunc = 0;
CODE_MMIOINIFILECODEC   CODE_mmioIniFileCODECFunc   = 0;
CODE_MMIOMIGRATEINIFILE CODE_mmioMigrateIniFileFunc = 0;

BOOL MINSTALL_LinkInImports (void) {
   // Remember this action for cleanup...
   MINSTALL_Done |= MINSTDONE_LINKINIMPORTS;

   if (!(MINSTIMP_MDMHandle = DLL_Load("MDM.DLL"))) {
      MINSTALL_TrappedError (MINSTMSG_CouldNotLinkIn); return FALSE; }
   MCIINI_MciSendCommandFunc = (CODE_MCISENDCOMMAND)DLL_GetEntryPoint(MINSTIMP_MDMHandle, "mciSendCommand");
   CODE_MciSetSysValue       = (CODE_MCISETSYSVALUE)DLL_GetEntryPoint(MINSTIMP_MDMHandle, "mciSetSysValue");
   if ((!MCIINI_MciSendCommandFunc) || (!CODE_MciSetSysValue)) {
      MINSTALL_TrappedError (MINSTMSG_CouldNotLinkIn); return FALSE; }
   if (!(MINSTIMP_SSMINIHandle = DLL_Load("SSMINI.DLL"))) {
      MINSTALL_TrappedError (MINSTMSG_CouldNotLinkIn); return FALSE; }
   CODE_SpiInstallFunc = (CODE_SPIINSTALL)DLL_GetEntryPoint(MINSTIMP_SSMINIHandle, "SpiInstall");
   if (!CODE_SpiInstallFunc) {
      MINSTALL_TrappedError (MINSTMSG_CouldNotLinkIn); return FALSE; }
   if (!(MINSTIMP_MMIOHandle = DLL_Load("MMIO.DLL"))) {
      MINSTALL_TrappedError (MINSTMSG_CouldNotLinkIn); return FALSE; }
   CODE_mmioIniFileHandlerFunc = (CODE_MMIOINIFILEHANDLER)DLL_GetEntryPoint(MINSTIMP_MMIOHandle, "mmioIniFileHandler");
   CODE_mmioIniFileCODECFunc   = (CODE_MMIOINIFILECODEC)DLL_GetEntryPoint(MINSTIMP_MMIOHandle, "mmioIniFileCODEC");
   CODE_mmioMigrateIniFileFunc = (CODE_MMIOMIGRATEINIFILE)DLL_GetEntryPoint(MINSTIMP_MMIOHandle, "mmioMigrateIniFile");
   if ((!CODE_mmioIniFileHandlerFunc) || (!CODE_mmioIniFileCODECFunc) || (!CODE_mmioMigrateIniFileFunc)) {
      MINSTALL_TrappedError (MINSTMSG_CouldNotLinkIn); return FALSE; }

   return TRUE;
 }

VOID MINSTALL_CleanUpImports (void) {
   if (MINSTIMP_MDMHandle)
      DLL_UnLoad (MINSTIMP_MDMHandle);
 }

VOID MINSTALL_MigrateMMPMMMIOFile (void) {
   // This is an undocumented API, that migrates current MMPMMMIO.INI file,
   //  if available or generates a new one.
   (*CODE_mmioMigrateIniFileFunc) (0);
 }

VOID MINSTALL_LowerMasterVolume (void) {
   // This is called on base-installations to set master volume to 60.
   (*CODE_MciSetSysValue) (1, 60);
 }
