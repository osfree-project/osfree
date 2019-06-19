
// щ Д ДДДДНН = Д  щ  Д = ННДДДД Д щ
// і                               і
//    ЬЫЫЫЫЫЫЫЬ   ЬЫЬ  ЬЫЫЫЫЫЫЫЫЬ          ъ  ъДДДНДДНДННДДННННДНННННННННОД
// і ЫЫЫЫЯЯЯЫЫЫЫ ЫЫЫЫЫ ЫЫЫЯ   ЯЫЫЫ і             MINSTALL Front-End      є
// є ЫЫЫЫЬЬЬЫЫЫЫ ЫЫЫЫЫ ЫЫЫЬ   ЬЫЫЫ є      ъ ДДДДНДННДДННННДННННННННДНННННОД
// є ЫЫЫЫЫЫЫЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫЫЫЫЫЫЯ  є       Section: MMOS/2 for eCS       є
// є ЫЫЫЫ   ЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫ ЯЫЫЫЫЬ є     і Created: 28/10/02             є
// і ЯЫЫЯ   ЯЫЫЯ  ЯЫЯ  ЯЫЫЯ   ЯЫЫЯ і     і Last Modified:                і
//                  ЬЬЬ                  і Number Of Modifications: 000  і
// щ              ЬЫЫЯ             щ     і INCs required: *none*         і
//      ДДДДДДД ЬЫЫЯ                     є Written By: Martin Kiewitz    і
// і     ЪїЪїіЬЫЫЫЬЬЫЫЫЬ           і     є (c) Copyright by              і
// є     АЩіАЩЯЫЫЫЯЯЬЫЫЯ           є     є      AiR ON-Line Software '02 ъ
// є    ДДДДДДД    ЬЫЫЭ            є     є All rights reserved.
// є              ЬЫЫЫДДДДДДДДД    є    ДОНННДНННННДННННДННДДНДДНДДДъДД  ъ
// є             ЬЫЫЫЭі іЪїііД     є
// і            ЬЫЫЫЫ АДііАЩіД     і
//             ЯЫЫЫЫЭДДДДДДДДДД
// і             ЯЯ                і
// щ Дґ-=’iз йп-Liпо SйџвW’зо=-ГДД щ

#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#define INCL_OS2MM
#include <os2.h>
//#include <pmdef.h>
//#include <os2me.h>
#include <mcios2.h>

#include <string.h>
#include <stdlib.h>

#include <global.h>
#include <mciini.h>


APIRET MCIINI_SendSysInfoExtCommand (ULONG FunctionID, PVOID SysInfoParm) {
   MCI_SYSINFO_PARMS SysInfo;

   memset (&SysInfo, 0, sizeof(SysInfo));
   SysInfo.ulItem       = FunctionID;
   SysInfo.pSysInfoParm = SysInfoParm;
   return (*MCIINI_MciSendCommandFunc) (0, MCI_SYSINFO, MCI_SYSINFO_ITEM|MCI_WAIT, (PVOID)&SysInfo, 0);
 }

APIRET MCIINI_CreateDriver (PSZ DeviceName, USHORT DeviceType, ULONG DeviceFlags, PSZ VersionNo, PSZ ProductInfo, PSZ MCDDriver, PSZ VSDDriver, PSZ PDDName, PSZ MCDTable, PSZ VSDTable, USHORT ShareType, PSZ ResourceName, USHORT ResourceUnits, USHORT ClassesCount, PUSHORT ClassArray, PUSHORT ClassComboArray) {
   MCI_SYSINFO_LOGDEVICE AddDriver;

   memset (&AddDriver, 0, sizeof(AddDriver));
   strncpy (AddDriver.szInstallName, DeviceName, MAX_DEVICE_NAME);
   AddDriver.usDeviceType      = DeviceType;
   AddDriver.ulDeviceFlag      = DeviceFlags;
   strncpy (AddDriver.szVersionNumber, VersionNo, MAX_VERSION_NUMBER);
   strncpy (AddDriver.szProductInfo, ProductInfo, MAX_PRODINFO);
   strncpy (AddDriver.szMCDDriver, MCDDriver, MAX_DEVICE_NAME);
   strncpy (AddDriver.szVSDDriver, VSDDriver, MAX_DEVICE_NAME);
   strncpy (AddDriver.szPDDName, PDDName, MAX_PDD_NAME);
   strncpy (AddDriver.szMCDTable, MCDTable, MAX_DEVICE_NAME);
   strncpy (AddDriver.szVSDTable, VSDTable, MAX_DEVICE_NAME);
   AddDriver.usShareType       = ShareType;
   strncpy (AddDriver.szResourceName, ResourceName, MAX_DEVICE_NAME);
   AddDriver.usResourceUnits   = ResourceUnits;
   AddDriver.usResourceClasses = ClassesCount;
   memcpy (&AddDriver.ausClassArray, ClassArray, sizeof(USHORT)*ClassesCount);
   memcpy (&AddDriver.ausValidClassArray, ClassComboArray, sizeof(USHORT)*10*10); // ClassesCount*ClassesCount);
   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_INSTALL_DRIVER, &AddDriver);
 }

APIRET MCIINI_DeleteDriver (PSZ DeviceName) {
   MCI_SYSINFO_LOGDEVICE DelDriver;

   memset  (&DelDriver, 0, sizeof(DelDriver));
   strncpy (DelDriver.szInstallName, DeviceName, MAX_DEVICE_NAME);
   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_DELETE_DRIVER, &DelDriver);
 }

APIRET MCIINI_GetDeviceParameters (PSZ DestPtr, ULONG DestMaxLen, PSZ DeviceName) {
   MCI_SYSINFO_DEVPARAMS GetDevParms;
   APIRET                rc;

   if (DestMaxLen==0) return FALSE;
   memset  (&GetDevParms, 0, sizeof(GetDevParms));
   memset  (DestPtr, 0, DestMaxLen);
   strncpy (GetDevParms.szInstallName, DeviceName, MAX_DEVICE_NAME);
   if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_QUERY_PARAMS, &GetDevParms))==0) {
      strncpy (DestPtr, GetDevParms.szDevParams, DestMaxLen);
    }
   return rc;
 }

APIRET MCIINI_SetDeviceParameters (PSZ DeviceName, PSZ DeviceParameters) {
   MCI_SYSINFO_DEVPARAMS SetDevParms;

   memset  (&SetDevParms, 0, sizeof(SetDevParms));
   strncpy (SetDevParms.szInstallName, DeviceName, MAX_DEVICE_NAME);
   strncpy (SetDevParms.szDevParams, DeviceParameters, MAX_DEV_PARAMS);
   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_PARAMS, &SetDevParms);
 }

USHORT MCIINI_GetConnectors (PUSHORT ConnectorTypeArrayPtr, PCHAR ToInstallNameArrayPtr, PUSHORT ToConnectIndexArrayPtr, USHORT MaxCount, PSZ DeviceName) {
   MCI_SYSINFO_CONPARAMS GetConnectors;
   USHORT                CurConnector;
   PCONNECT              CurConnectorPtr;

   if (MaxCount==0) return FALSE;
   memset  (&GetConnectors, 0, sizeof(GetConnectors));
   strncpy (GetConnectors.szInstallName, DeviceName, MAX_DEVICE_NAME);
   if ((MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_QUERY_CONNECTORS, &GetConnectors))==0) {
      if (MaxCount>GetConnectors.usNumConnectors)
         MaxCount = GetConnectors.usNumConnectors;

      CurConnectorPtr = &GetConnectors.ConnectorList[0];
      for (CurConnector=0; CurConnector<MaxCount; CurConnector++) {
         *ConnectorTypeArrayPtr = CurConnectorPtr->usConnectType;
         memcpy (ToInstallNameArrayPtr, &CurConnectorPtr->szToInstallName, MAX_DEVICE_NAME);
         *ToConnectIndexArrayPtr = CurConnectorPtr->usToConnectIndex;
         CurConnectorPtr++;
         ConnectorTypeArrayPtr++; ToConnectIndexArrayPtr++;
         ToInstallNameArrayPtr += MAX_DEVICE_NAME;
       }
      return GetConnectors.usNumConnectors;
    }
   return 0;

 }

APIRET MCIINI_SetConnectors (PSZ DeviceName, USHORT ConnectorCount, PUSHORT ConnectorTypeArrayPtr, PCHAR ToInstallNameArrayPtr, PUSHORT ToConnectIndexArrayPtr) {
   MCI_SYSINFO_CONPARAMS SetConnectors;
   USHORT                CurConnector;
   PCONNECT              CurConnectorPtr;
   APIRET                rc;

   memset  (&SetConnectors, 0, sizeof(SetConnectors));
   strncpy (SetConnectors.szInstallName, DeviceName, MAX_DEVICE_NAME);
   SetConnectors.usNumConnectors = ConnectorCount;

   if (ConnectorCount>MAX_CONNECTORS) ConnectorCount = MAX_CONNECTORS;
   // Copy all connectors from 3 arrays into one...
   CurConnectorPtr = &SetConnectors.ConnectorList[0];
   for (CurConnector=0; CurConnector<ConnectorCount; CurConnector++) {
      CurConnectorPtr->usConnectType    = *ConnectorTypeArrayPtr;
      memcpy (&CurConnectorPtr->szToInstallName, ToInstallNameArrayPtr, MAX_DEVICE_NAME);
      CurConnectorPtr->usToConnectIndex = *ToConnectIndexArrayPtr;
      CurConnectorPtr++;
      ConnectorTypeArrayPtr++; ToConnectIndexArrayPtr++;
      ToInstallNameArrayPtr += MAX_DEVICE_NAME;
    }

   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_CONNECTORS, &SetConnectors);
 }

USHORT MCIINI_GetFileExtensions (PCHAR ExtensionArrayPtr, USHORT MaxCount, PSZ DeviceName) {
   MCI_SYSINFO_EXTENSION GetExtensions;

   if (MaxCount==0) return FALSE;
   memset  (&GetExtensions, 0, sizeof(GetExtensions));
   strncpy (GetExtensions.szInstallName, DeviceName, MAX_DEVICE_NAME);
   if ((MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_QUERY_EXTENSIONS, &GetExtensions))==0) {
      if (MaxCount>GetExtensions.usNumExtensions)
         MaxCount = GetExtensions.usNumExtensions;
      memcpy (ExtensionArrayPtr, GetExtensions.szExtension, MaxCount*MAX_EXTENSION_NAME);
      return GetExtensions.usNumExtensions;
    }
   return 0;
 }

APIRET MCIINI_SetFileExtensions (PSZ DeviceName, USHORT ExtensionCount, PCHAR ExtensionArrayPtr) {
   MCI_SYSINFO_EXTENSION SetExtensions;

   memset (SetExtensions.szExtension, 0, sizeof(SetExtensions.szExtension));

   strncpy (SetExtensions.szInstallName, DeviceName, MAX_DEVICE_NAME);
   if (ExtensionCount>MAX_EXTENSIONS) ExtensionCount = MAX_EXTENSIONS;
   SetExtensions.usNumExtensions = ExtensionCount;
   memcpy (SetExtensions.szExtension, ExtensionArrayPtr, ExtensionCount*MAX_EXTENSION_NAME);
   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_EXTENSIONS, &SetExtensions);
 }

APIRET MCIINI_GetAliasName (PSZ DestPtr, ULONG DestMaxLen, PSZ DeviceName) {
   MCI_SYSINFO_QUERY_NAME GetAlias;
   APIRET                 rc;

   if (DestMaxLen==0) return FALSE;
   memset  (&GetAlias, 0, sizeof(GetAlias));
   memset  (DestPtr, 0, DestMaxLen);
   strncpy (GetAlias.szInstallName, DeviceName, MAX_DEVICE_NAME);
   if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_QUERY_NAMES, &GetAlias))==0) {
      strncpy (DestPtr, GetAlias.szAliasName, DestMaxLen);
    }
   return rc;
 }

APIRET MCIINI_SetAliasName (PSZ DeviceName, PSZ AliasName) {
   MCI_SYSINFO_ALIAS SetAlias;

   memset  (&SetAlias, 0, sizeof(SetAlias));
   strncpy (SetAlias.szInstallName, DeviceName, MAX_DEVICE_NAME);
   strncpy (SetAlias.szAliasName, AliasName, MAX_ALIAS_NAME);
   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_ALIAS, &SetAlias);
 }

APIRET MCIINI_GetEATypes (PSZ DestPtr, ULONG DestMaxLen, PSZ DeviceName) {
   MCI_SYSINFO_TYPES GetEATypes;
   APIRET            rc;

   if (DestMaxLen==0) return FALSE;
   memset  (&GetEATypes, 0, sizeof(GetEATypes));
   memset  (DestPtr, 0, DestMaxLen);
   strncpy (GetEATypes.szInstallName, DeviceName, MAX_DEVICE_NAME);
   if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_QUERY_TYPES, &GetEATypes))==0) {
      strncpy (DestPtr, GetEATypes.szTypes, DestMaxLen);
    }
   return rc;
 }

APIRET MCIINI_SetEATypes (PSZ DeviceName, PSZ EATypes) {
   MCI_SYSINFO_TYPES SetEATypes;

   memset  (&SetEATypes, 0, sizeof(SetEATypes));
   strncpy (SetEATypes.szInstallName, DeviceName, MAX_DEVICE_NAME);
   strncpy (SetEATypes.szTypes, EATypes, MAX_TYPEBUFFER);
   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_TYPES, &SetEATypes);
 }

APIRET MCIINI_GetDefaultDevice (PSZ DestPtr, ULONG DestMaxLen, USHORT DeviceType) {
   MCI_SYSINFO_DEFAULTDEVICE GetDefaultDevice;
   APIRET                    rc;

   memset (&GetDefaultDevice, 0, sizeof(GetDefaultDevice));
   GetDefaultDevice.usDeviceType = DeviceType;
   if ((rc = MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_QUERY_DEFAULT, &GetDefaultDevice))==0) {
      strncpy (DestPtr, GetDefaultDevice.szInstallName, DestMaxLen);
    }
   return rc;
 }

APIRET MCIINI_SetDefaultDevice (USHORT DeviceType, PSZ DeviceName) {
   MCI_SYSINFO_DEFAULTDEVICE SetDefaultDevice;
   memset (&SetDefaultDevice, 0, sizeof(SetDefaultDevice));
   strncpy (SetDefaultDevice.szInstallName, DeviceName, MAX_DEVICE_NAME);
   SetDefaultDevice.usDeviceType = DeviceType;
   return MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_SET_DEFAULT, &SetDefaultDevice);
 }

USHORT MCIINI_GetDeviceOrdinal (PSZ DeviceName) {
   MCI_SYSINFO_QUERY_NAME QueryName;
   APIRET rc;

   memset  (&QueryName, 0, sizeof(QueryName));
   strncpy (QueryName.szInstallName, DeviceName, MAX_DEVICE_NAME);
   if (!MCIINI_SendSysInfoExtCommand(MCI_SYSINFO_QUERY_NAMES, &QueryName))
      return 0;                             // if any error occured...
   return QueryName.usDeviceOrd;
 }

USHORT MCIINI_GetTotalDevices (USHORT DeviceType) {
   MCI_SYSINFO_PARMS SysInfo;
   CHAR              ReturnBuffer[4];

   memset (&SysInfo, 0, sizeof(SysInfo));
   memset (&ReturnBuffer, 0, sizeof(ReturnBuffer));
   SysInfo.pszReturn    = ReturnBuffer;
   SysInfo.ulRetSize    = 3;
   SysInfo.usDeviceType = DeviceType;
   if ((*MCIINI_MciSendCommandFunc) (0, MCI_SYSINFO, MCI_TO_BUFFER, (PVOID)&SysInfo, 0))
      return 0;                             // Error during MCI-Command...
   return atoi(ReturnBuffer);
 }
