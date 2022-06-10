/* $Id: 1$ */
/** @acpi.c
 *
 * API interface for ACPI
 *
 * netlabs.org confidential
 *
 * Copyright (c) 2005 netlabs.org
 * Copyright (c) 2011-2012 Mensys, BV
 * Copyright (c) 2011-2014 David Azarewicz <david@88watts.net>
 *
 * Author: Pavel Shtemenko <pasha@paco.odessa.ua>
 * Modified by: David Azarewicz <david@88watts.net>
 *
 * All Rights Reserved
 */
#ifndef __ACPIAPI_H__
#define __ACPIAPI_H__

#pragma pack(1)

#define DRV_NAME "ACPICA$"
#define ACPI_TK_VERSION_MAJOR 1
#define ACPI_TK_VERSION_MINOR 2

typedef struct _VersionAcpi_
{
    ULONG                   Major;
    ULONG                   Minor;

} ACPI_VERSION;

typedef struct _AcpiApiHandle_
{
    HFILE                   AcpiDrv;                       // Handle to ACPICA driver
    ACPI_VERSION            PSD;                           // Version PSD
    ACPI_VERSION            Driver;                        // Version ACPICA driver
    ACPI_VERSION            DLL;                           // Version acpi32.dll
    ULONG                   StartAddrPSD;                  // Start address PSD (for testcase)
    ULONG                   AddrCommApp;                   // Address DosCommApp from PSD (which not write IBM)
    ULONG                   StartAddrDriver;               // Start address ACPICA (for testcase)
    ULONG                   AddrFindPSD;                   // Address function for find PSD (find CommApp)
    ULONG                   IRQNumber;                     // Number use IRQ
    void                    *Internal;                     // For internal DLL use
} ACPI_API_HANDLE, *PACPI_API_HANDLE;

typedef struct _LAPICError_
{
    UINT32                  SendChecksum;
    UINT32                  ReceiveChecksum;
    UINT32                  SendAccept;
    UINT32                  ReceiveAccept;
    UINT32                  SendIllegal;
    UINT32                  ReceiveIllegal;
    UINT32                  IllegalRegAddress;
} LAPICError, *PLAPICError;

typedef struct _lint_
{
    UINT8   Valid;
    UINT8   Type;
    UINT8   Polarity;
    UINT8   Trigger;
} LINT_PIN;

#define MAX_CSTATE  3
#define LINT_TYPE_NMI 1

typedef struct _Processor_
{
    ACPI_IO_ADDRESS         PblkAddress;                   // Address P_BLK
    UINT32                  PblkLength;                    // Length  P_BLK
    UINT8                   Width;                         // Width of trottling
    UINT8                   Offset;                        // ofsset from begin
    UINT8                   Flags;                         // Special use flags
    UINT8                   LocalApicId;                   // Field from MADT
    UINT32                  ProcessorId;                   // Field from MADT
    LAPICError              APIC;                          // Local APIC error counter
    ACPI_HANDLE             Handle;                        // Handle to ACPI tree
    ACPI_GENERIC_ADDRESS    PerfControl;                   // Perfomance control
    ACPI_GENERIC_ADDRESS    PerfStatus;                    // Perfomance control
    UINT32                  CxLatency[MAX_CSTATE];         // C state latency
    ACPI_GENERIC_ADDRESS    CxState[MAX_CSTATE];           // C state control register
    UINT32                  CR3Addr;                       // Phys addr of PageDir
    UINT32                  LastThrtl;                     // Last setting throttling
    LINT_PIN                Lint[2];
    // Statistics
    UINT32                  HaltCounter;
    UINT32                  IPIPsdCount;
    UINT64                  IPIGenCount;
    UINT64                  IPICount;
    UINT64                  IdleCounter;
    UINT64                  BusyCounter;
} PROCESSOR, *PPROCESSOR;

typedef void   (*EOIFunction)(void);
typedef void   (*KernelFunction)(void);
typedef void   (*MaskFunction)(UINT32 IRQ,UINT32 Flag);
typedef UINT32 (*IxRFunction )(UINT32 IRQ,UINT32 Flag);
typedef struct _IRQ_
{
    EOIFunction             fEOI;                          // EOI for this Irq i
    UINT64                  Count;                         // EOI Called count for Irq i
    UINT32                  Triggering;                    // Edge/Level for Irq i
    UINT32                  Polarity;                      // High/Low for Irq i
    UINT32                  Sharable;                      // This Irq i is sharable Yes/No
    UINT32                  Vector;                        // Vector for IDT for Irq i
    volatile UINT32         *IOApicAddress;                // Address of IO APIC for this Irq i
    UINT32                  BusType;                       // Type of bus (ie. PCI) for Irq i
    UINT32                  Line;                          // Line number for selecting IO APIC register (!= to Irq i)
    UINT32                  IrqLowWord;                    // Low dword for IO APIC for Irq i
    UINT32                  IrqHighWord;                   // High dword for IO APIC for Irq i
    void                    *pAdapter;
    KernelFunction          OS2handler;                    // OS/2 IRQ handler for Irq i
    MaskFunction            fMask;                         // Function to mask/unmask IRQ for Irq i
    IxRFunction             fIxR;                          // Function to get ISR/IRR for Irq i
} AcpiIRQ,   *PAcpiIRQ;

#define BUS_TYPE_PCI 1
#define BUS_TYPE_MSI 2
#define BUS_TYPE_MSIX 4

typedef struct _IOAPIC_
{
    UINT32                  PhysAddr;                      // Physical address from MADT
    volatile UINT32         *LinAddr;                      // Linear address
    UINT32                  BaseIRQ;                       // Start IRQ from. getting from MADT
    UINT8                   IOApicId;                      // ID from MADT
    UINT8                   IOApicIdOrig;                  // ID from MADT
    UINT8                   Version;                       // Chip version
    UINT8                   Arbitration;                   // Chip arbitration
    UINT8                   Lines;                         // How many lines in this chip
    UINT8                   Unused[3];
} IOAPIC, *PIOAPIC;

typedef struct _ACPI_STATISTICS_
{
    UINT32                  Command;
    UINT32                  Number;
    UINT32                  Status;
    PVOID                   Data;
}  OS2_ACPI_STATISTICS, *POS2_ACPI_STATISTICS;

typedef struct _ACPI_Notify_
{
    UINT32                  Number;                        // Message Number
    ACPI_HANDLE             Handle;                        // Handle to ACPI device
} ACPINotify, *PACPINotify;

#define ACPI_STATISTICS_GETSIZEIRQ    1
#define ACPI_STATISTICS_GETIRQ        2
#define ACPI_STATISTICS_CLEARIRQ      3
#define ACPI_STATISTICS_GETSIZECPU    4
#define ACPI_STATISTICS_GETCPU        5
#define ACPI_STATISTICS_GETPSDINFO    6

/* WARNING: This structure is OBSOLETE and should never be used.
 * This structure may be removed in a future release.
 */
typedef struct _KnownDevice_
{
    ACPI_PCI_ID PciId;
    UINT32      PicIrq;
    UINT32      ApicIrq;
    ACPI_HANDLE  Handle;
} KNOWNDEVICE, *PKNOWNDEVICE;

typedef struct _ButtonEvent_
{
    UINT32                  ButtonStatus;
    UINT32                  WhichButton;
} BUTTONEVENT, *PBUTTONEVENT;
//---------------------------------------------------------------------
// Rudi:  additional IOCTL data structures

typedef struct
{
    ULONG                   ulADDMajor;
    ULONG                   ulADDMinor;
    ULONG                   ulADDRsvd1;
    ULONG                   ulPSDRevNum;
    ULONG                   ulPSDMajor;
    ULONG                   ulPSDMinor;
    ULONG                   ulPSDRsvd1;
    ULONG                   ulPSD_APP_COMM;
    ULONG                   ulIntNum;
} ACPI_VERSION_REQ;


typedef struct
{
    ULONGLONG               ullPMCounter;
    ULONG                   ulAcpiTimer;
} ACPI_TIMER_REQ;


typedef struct
{
    ULONG                   ulTimeout;                     // on return:  button state
    ULONG                   ulButtonID;                    // on return:  error code
} ACPI_BUTTON_REQ;

// Rudi:  additional IOCTL data structures end
//---------------------------------------------------------------------

typedef struct _AcpiTkVersion
{
    ULONG Size;
    ACPI_VERSION PSD;     // Version PSD
    ACPI_VERSION Driver;  // Version ACPICA driver
    ACPI_VERSION DLL;     // Version acpi32.dll
    ACPI_VERSION Api;     // Version of the API interface
} ACPI_TK_VERSION;

typedef struct _Os2PsdInfo
{
    ULONG Size;
    ULONG RevNumber;
    ULONG IntMode;
    ULONG DebugLevel;
    CHAR KernelName[32];
} OS2_PSD_INFO, *POS2_PSD_INFO;

#ifdef __cplusplus
extern "C" {
#endif

ACPI_STATUS APIENTRY AcpiTkGetVersion(ACPI_TK_VERSION *ApiBuffer);
ACPI_STATUS APIENTRY AcpiTkValidateVersion(UINT32 TestMajor, UINT32 TestMinor);
ACPI_STATUS APIENTRY AcpiTkGetTimerResolution(UINT32 *Resolution);
ACPI_STATUS APIENTRY AcpiTkGetTimer(UINT32 *Ticks);
ACPI_STATUS APIENTRY AcpiTkGetNextObject(ACPI_OBJECT_TYPE Type, ACPI_HANDLE Parent, ACPI_HANDLE Child, ACPI_HANDLE *OutHandle);
ACPI_STATUS APIENTRY AcpiTkGetParent(ACPI_HANDLE Object, ACPI_HANDLE *OutHandle);
ACPI_STATUS APIENTRY AcpiTkGetType(ACPI_HANDLE Object, ACPI_OBJECT_TYPE *OutType);
ACPI_STATUS APIENTRY AcpiTkWalkNamespace(ACPI_OBJECT_TYPE Type, ACPI_HANDLE StartObject, UINT32 MaxDepth,
    ACPI_WALK_CALLBACK UserFunction, void *Context, void **ReturnValue);
ACPI_STATUS APIENTRY AcpiTkRsGetMethodData(ACPI_HANDLE Handle, char *Path, ACPI_BUFFER *RetBuffer);
ACPI_STATUS APIENTRY AcpiTkWalkResources(ACPI_HANDLE DeviceHandle, char *Name, ACPI_WALK_RESOURCE_CALLBACK UserFunction, void *Context);
//ACPI_STATUS APIENTRY AcpiTkGetObjectInfo (ACPI_HANDLE Handle, ACPI_BUFFER *ReturnBuffer);
ACPI_STATUS APIENTRY AcpiTkGetObjectInfoAlloc3(ACPI_HANDLE Handle, ACPI_DEVICE_INFO **ReturnBuffer);
void APIENTRY AcpiTkOsFree(void *MemAddr);
ACPI_STATUS APIENTRY AcpiTkGetName(ACPI_HANDLE Handle, UINT32 NameType, ACPI_BUFFER *RetPathPtr);
ACPI_STATUS APIENTRY AcpiTkGetTable(ACPI_STRING Signature, UINT32 Instance, ACPI_TABLE_HEADER **OutTable);
ACPI_STATUS APIENTRY AcpiTkGetTableHeader(ACPI_STRING Signature, UINT32 Instance, ACPI_TABLE_HEADER *OutTableHeader);
ACPI_STATUS APIENTRY AcpiTkGetSystemInfo(ACPI_BUFFER *RetBuffer);
ACPI_STATUS APIENTRY AcpiTkEvaluateObject(ACPI_HANDLE Object, ACPI_STRING Pathname,
    ACPI_OBJECT_LIST *ParameterObjects, ACPI_BUFFER *ReturnObjectBuffer);
ACPI_STATUS APIENTRY AcpiTkGetHandle(ACPI_HANDLE Parent, ACPI_STRING Pathname, ACPI_HANDLE *RetHandle);
ACPI_STATUS APIENTRY AcpiTkEvaluateObjectTyped(ACPI_HANDLE Object, ACPI_STRING Pathname,
    ACPI_OBJECT_LIST *ExternalParams, ACPI_BUFFER *ReturnBuffer, ACPI_OBJECT_TYPE ReturnType);
ACPI_STATUS APIENTRY AcpiTkStartApi(ACPI_API_HANDLE *Hdl);
ACPI_STATUS APIENTRY AcpiTkEndApi(ACPI_API_HANDLE *Hdl);
ACPI_STATUS APIENTRY AcpiTkWaitPressButton(ACPI_API_HANDLE *Hdl, ULONG Which, ULONG TimeForWait, ULONG *State);
ACPI_STATUS APIENTRY AcpiTkGetUpTime(ACPI_API_HANDLE *Hdl, UINT64 *Called, UINT32 *Ticks);
ACPI_STATUS APIENTRY AcpiTkSetThrottling(ACPI_API_HANDLE *Hdl, UINT32 ProcId, UINT32 Perfomance);
ACPI_STATUS APIENTRY AcpiTkGetThrottling(ACPI_API_HANDLE *Hdl, UINT32 ProcId, UINT32 *Perfomance, UINT8 *Width);
ACPI_STATUS APIENTRY AcpiTkSetPowerState(ACPI_API_HANDLE *Hdl, UINT32 ProcId, UINT32 State);
const char * APIENTRY AcpiTkStatusToStr(ACPI_STATUS Status);
ACPI_STATUS APIENTRY AcpiTkGetStatistics(ACPI_API_HANDLE *Hdl, void *Buffer);
ACPI_STATUS APIENTRY AcpiTkGoToSleep(ACPI_API_HANDLE *Hdl, UINT8 State);
ACPI_STATUS APIENTRY AcpiTkEC(ACPI_API_HANDLE *Hdl);
ACPI_STATUS APIENTRY AcpiTkWaitNotify(ACPI_API_HANDLE *Hdl, ACPI_HANDLE *NotifyHandle, UINT32 *NotifyNumber, UINT32 What, UINT32 TimeOut);
ACPI_STATUS APIENTRY AcpiTkWaitEmbeddedEvent(ACPI_API_HANDLE *Hdl, ACPI_HANDLE *EmbeddedHandle, UINT32 *EmbeddedNumber, UINT32 TimeOut);
ACPI_STATUS APIENTRY AcpiTkGetPCIDev(ACPI_API_HANDLE *Hdl, PKNOWNDEVICE dev);
ACPI_STATUS APIENTRY AcpiTkRWEmbedded(ACPI_API_HANDLE *Hdl, UINT32 Number, UINT32 Function, ACPI_PHYSICAL_ADDRESS Address, ACPI_INTEGER *Value);
ACPI_STATUS APIENTRY AcpiTkPCIFunction(ACPI_API_HANDLE *Hdl, UINT32 Function, ACPI_PCI_ID *PciId, UINT32 Register, UINT64 *Value, UINT32 Width);
#define PCI_FUNCTION_READ    0
#define PCI_FUNCTION_WRITE   1
ACPI_STATUS APIENTRY AcpiTkInternalFunction(PVOID Parm, UINT32 ParmSize);
#define INTERNAL_FUNCTION_DPRINT 1
#define INTERNAL_FUNCTION_RESET 2
#define INTERNAL_FUNCTION_FADTFLAGS 3
const char * APIENTRY AcpiTkUtGetTypeName(ACPI_OBJECT_TYPE Type);
#define NOTIFY_TYPE_SLEEPRESTORE 0
#define NOTIFY_FUNCTION_SLEEP 0
#define NOTIFY_FUNCTION_RESTORE 1
ACPI_STATUS APIENTRY AcpiTkNotifyDrivers(ACPI_API_HANDLE *Hdl, UINT32 Type, UINT32 Function);
ACPI_STATUS APIENTRY AcpiTkPrepareToSleep(UINT8 State);

#ifdef __cplusplus
}
#endif

#define WAIT_POWER_FOREVER             -1
#define SET_POWERBUTTON                0x100               // Wait Power Button
#define SET_SLEEPBUTTON                0x101               // Wait Sleep Button
#define ERROR_WRONG_VERSION            0xC0000001          // Wrong version
// For PMtimer use
#define PM_TIMER_FREQUENCY_SEC         3579545
#define PM_TIMER_FREQUENCY_MSEC        3580
#define PM_TIMER_FREQUENCY_10MSEC      35795
#define PM_TIMER_FREQUENCY_20MSEC      35795 * 2
#define PM_TIMER_FREQUENCY_MkSEC       4
#define PM_TIMER_FREQUENCY_2MkSEC      7
#define PM_TIMER_FREQUENCY_10MkSEC     36
#define PM_TIMER_FREQUENCY_20MkSEC     72
#define PM_TIMER_FREQUENCY_100MkSEC    358

#pragma pack()

#endif

