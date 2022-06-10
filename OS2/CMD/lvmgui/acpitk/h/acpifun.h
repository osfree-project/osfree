/** @acpi.c
 *
 * Exported function structure for ACPI
 *
 * netlabs.org confidential
 *
 * Copyright (c) 2006 netlabs.org
 * Copyright (c) 2011-2012 Mensys, BV
 * Copyright (c) 2011-2016 David Azarewicz <david@88watts.net>
 *
 * Author: Pavel Shtemenko <pasha@paco.odessa.ua>
 * Modified by: David Azarewicz <david@88watts.net>
 *
 * All Rights Reserved
 *
 */
#ifndef __ACPIFUN_H__
#define __ACPIFUN_H__

#pragma pack(1)

#ifdef __WATCOMC__
#define PSDCALL  __stdcall
#define PSDENTRY __stdcall
#else
#define PSDCALL  _Stdcall
#define PSDENTRY _Stdcall
#endif
#define PSDRET   int

typedef PSDRET (PSDENTRY *P_F_1)(ULONG arg);
typedef PSDRET (PSDENTRY *P_F_2)(ULONG arg1, ULONG arg2);

#ifndef __ACTYPES_H__
typedef unsigned long ACPI_STATUS;
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int                          (*ACPIMSIALLOC               )(ACPI_PCI_ID *PciId, ULONG *pCount, ULONG *pulIrq);
typedef int                          (*ACPIMSIXALLOC              )(ACPI_PCI_ID *PciId, ULONG *pCount, ULONG *pulIrq);
typedef int                          (*ACPIMSIRELEASE             )(ACPI_PCI_ID *PciId);
typedef ACPI_STATUS                  (*ACPIEVALUATEOBJECT         )(ACPI_HANDLE Object,ACPI_STRING  Pathname, ACPI_OBJECT_LIST *ParameterObjects, ACPI_BUFFER *ReturnObjectBuffer);
typedef ACPI_STATUS                  (*ACPIWALKNAMESPACE          )(ACPI_OBJECT_TYPE Type,ACPI_HANDLE StartObject,UINT32 MaxDepth,ACPI_WALK_CALLBACK PreUserFunction,ACPI_WALK_CALLBACK PostUserFunction,void *Context,void **ReturnValue);
typedef ACPI_STATUS                  (*ACPIGETDEVICES             )(char *HID,ACPI_WALK_CALLBACK UserFunction,void *Context,void **ReturnValue);
typedef ACPI_STATUS                  (*ACPIGETNAME                )(ACPI_HANDLE Handle,UINT32 NameType,ACPI_BUFFER *RetPathPtr);
typedef ACPI_STATUS                  (*ACPIGETHANDLE              )(ACPI_HANDLE Parent,ACPI_STRING Pathname,ACPI_HANDLE *RetHandle);
typedef ACPI_STATUS                  (*ACPIGETOBJECTINFO          )(ACPI_HANDLE Handle,ACPI_DEVICE_INFO **ReturnBuffer);
typedef ACPI_STATUS                  (*ACPIGETTYPE                )(ACPI_HANDLE Object,ACPI_OBJECT_TYPE *OutType);
typedef ACPI_STATUS                  (*ACPIGETNEXTOBJECT          )(ACPI_OBJECT_TYPE Type, ACPI_HANDLE Parent, ACPI_HANDLE Child, ACPI_HANDLE *OutHandle);
typedef ACPI_STATUS                  (*ACPIGETPARENT              )(ACPI_HANDLE Object, ACPI_HANDLE *OutHandle);
typedef const char                  *(*ACPIFORMATEXEPTION         )(ACPI_STATUS Exception);
typedef void                         (*ACPIPUTC                   )(UINT32 Char);
typedef void (ACPI_INTERNAL_VAR_XFACE *ACPIOSPRINTF               )(const char *Format, ...);
typedef ACPI_STATUS                  (*ACPIOSREADPCICONFIGURATION )(ACPI_PCI_ID *PciId,UINT32 Register, UINT64 *Value, UINT32 Width);
typedef ACPI_STATUS                  (*ACPIOSWRITEPCICONFIGURATION)(ACPI_PCI_ID *PciId,UINT32 Register, UINT64  Value, UINT32 Width);
typedef void                        *(*ACPIOSALLOCATE             )(ACPI_SIZE Size);
typedef void                         (*ACPIOSFREE                 )(void *Memory);
typedef ACPI_STATUS                  (*ACPIGETCURRENTRESOURCES    )(ACPI_HANDLE DeviceHandle, ACPI_BUFFER *RetBuffer);
typedef ACPI_STATUS                  (*ACPIGETPOSSIBLERESOURCES   )(ACPI_HANDLE DeviceHandle, ACPI_BUFFER *RetBuffer);
typedef ACPI_STATUS                  (*ACPISETCURRENTRESOURCES    )(ACPI_HANDLE DeviceHandle, ACPI_BUFFER  *InBuffer);
typedef ACPI_STATUS                  (*ACPIENTERSLEEPSTATEPREP    )(UINT8 SleepState);
typedef ACPI_STATUS                  (*ACPIENTERSLEEPSTATE        )(UINT8 SleepState);
typedef ACPI_STATUS                  (*ACPIENTERSLEEPSTATES4BIOS  )(void);
typedef ACPI_STATUS                  (*ACPILEAVESLEEPSTATE        )(UINT8 SleepState);
typedef ACPI_STATUS                  (*SETTHROTTLINGORCSTATE      )(PTHROTTLING_C_STATE State);
typedef ACPI_STATUS                  (*ACPIGETTIMER               )(UINT32  *Ticks);
typedef ACPI_STATUS                  (*ACPIGETTIMERDURATION       )(UINT32  StartTicks, UINT32  EndTicks, UINT32 *TimeElapsed);
typedef ACPI_STATUS                  (*ACPIWALKRESOURCES          )(ACPI_HANDLE DeviceHandle, char *Name, ACPI_WALK_RESOURCE_CALLBACK UserFunction, void *Context);
typedef ULONG                        (PSDCALL *PSDAPPCOM          )(APPCOMMPAR *par);
typedef ULONG                        (APIENTRY *APIERROR          )(void);

#ifdef __cplusplus
        }
#endif

typedef struct _ACPI_FUNCTION_
{
    ACPIMSIALLOC                AcpiMsiAlloc;
    ACPIMSIXALLOC               AcpiMsixAlloc;
    ACPIEVALUATEOBJECT          AcpiEvaluateObject;
    ACPIWALKNAMESPACE           AcpiWalkNamespace;
    ACPIGETDEVICES              AcpiGetDevices;
    ACPIGETNAME                 AcpiGetName;
    ACPIGETHANDLE               AcpiGetHandle;
    ACPIGETOBJECTINFO           AcpiGetObjectInfo;
    ACPIGETTYPE                 AcpiGetType;
    ACPIGETNEXTOBJECT           AcpiGetNextObject;
    ACPIGETPARENT               AcpiGetParent;
    ACPIFORMATEXEPTION          AcpiFormatException;
    ACPIMSIRELEASE              AcpiMsiRelease;
    ACPIPUTC                    AcpiPutc;
    ACPIOSPRINTF                AcpiOsPrintf;
    ACPIOSREADPCICONFIGURATION  AcpiOsReadPciConfiguration;
    ACPIOSWRITEPCICONFIGURATION AcpiOsWritePciConfiguration;
    ACPIOSALLOCATE              AcpiOsAllocate;
    ACPIOSFREE                  AcpiOsFree;
    ACPIGETCURRENTRESOURCES     AcpiGetCurrentResources;
    ACPIGETPOSSIBLERESOURCES    AcpiGetPossibleResources;
    ACPISETCURRENTRESOURCES     AcpiSetCurrentResources;
    ACPIENTERSLEEPSTATEPREP     AcpiEnterSleepStatePrep;
    ACPIENTERSLEEPSTATE         AcpiEnterSleepState;
    ACPIENTERSLEEPSTATES4BIOS   AcpiEnterSleepStateS4bios;
    ACPILEAVESLEEPSTATE         AcpiLeaveSleepState;
    APIERROR                    Unused1;
    SETTHROTTLINGORCSTATE       SetThrottlingOrCState;
    ACPIGETTIMERDURATION        AcpiGetTimerDuration;
    ACPIGETTIMER                AcpiGetTimer;
    ACPIWALKRESOURCES           AcpiWalkResources;
    APIERROR                    Unused2;
    PSDAPPCOM                   PSD_APP_COM;
    APIERROR                    Unused3;
    APIERROR                    Unused4;
} ACPIFUNCTION, *PACPIFUNCTION;

typedef struct _AcpiExportedFunction_
{
    UINT32                  Size;
    ACPIFUNCTION            Table;
} ACPIEXPORTEDFUNCTION, *PACPIEXPORTEDFUNCTION;

#pragma pack()

#endif
