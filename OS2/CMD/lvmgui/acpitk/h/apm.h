/* $Id: 1$ */
/** @apm.h
  *
  *  Header APM
  *
  * SSI confidential
  *
  * Copyright (c) 2006  Serenity Systems International
  *
  * Author: Pavel Shtemenko <pasha@paco.odessa.ua>
  *
  * All Rights Reserved
  *
  */
#pragma pack(1)

typedef struct _RegDDRtn
{
    USHORT                  Handle;                 /* Registration Handle Returned */
    PFN                     EventHandler;           /* Our APM Event Handler */
    ULONG                   NotifyMask;             /* Mask of Events to be notified of */
    USHORT                  ClientDS;               /* Our DS Selector */
    USHORT                  DeviceID;               /* Devices to be notified of */
} RegDDRtn;

typedef struct _DeRegDDRtn
{
    USHORT                  Handle;                 /* Registration Handle Returned */
} DeRegDDRtn;
typedef struct _SendPowerEvent
{
    ULONG                   Par1;
    ULONG                   Par2;
} SendPowerEvent;
typedef struct _QueryPowerStatus
{
    USHORT                  ParmLen;
    USHORT                  PowerFlag;
    UCHAR                   ACStatus;
    UCHAR                   BatteryStatus;
    UCHAR                   BatteryLife;
    UCHAR                   BatteryTimeForm;
    USHORT                  BatteryTime;
    USHORT                  BatteryFlags;
} QueryPowerStatus;
typedef struct _QueryPowerInformation
{
    USHORT                  ParmLen;
    USHORT                  BIOSFlag;
    USHORT                  BIOSVersion;
    USHORT                  SubsysVersion;
    USHORT                  APMVersion;
} QueryPowerInformation;
typedef struct _QueryPowerState
{
    USHORT                  ParmLen;
    USHORT                  DeviceId;
    USHORT                  PowerState;
} QueryPowerState;

typedef struct _OEMAPMFun
{
    USHORT                  ParmLen;
    USHORT                  Flags;
    ULONG                   EAX;
    ULONG                   EBX;
    ULONG                   ECX;
    ULONG                   EDX;
    ULONG                   ESI;
    ULONG                   EDI;
    USHORT                  DS;
    USHORT                  ES;
    USHORT                  FS;
    USHORT                  GS;
} OEMAPMFun;



typedef union _APMIDCcall_
{
    RegDDRtn                Register;
    DeRegDDRtn              DeRegister;
    SendPowerEvent          Event;
    QueryPowerStatus        Status;
    QueryPowerInformation   Information;
    QueryPowerState         State;
    OEMAPMFun               OEMFuntion;
} APMIDCcall;

typedef struct _IDCAPMFunction_
{
    USHORT                  Function;               /* APM IDC service      */
    APMIDCcall              Body;
} IDCAPMFunction, *pIDCAPMFunction;

// APM call device
typedef struct _IDCNotify
              {
              USHORT      NotifyFunction;      // Docs say "always 0" ????
              USHORT      SubID;
              USHORT      Reserved;
              USHORT      DevID;
              USHORT      PwrState;
} APMIDCNotify,  *PAPMIDCNotify;

typedef struct _NotificationDevice_
{
    APMIDCNotify            NotifyPacket;
    PFN                     EventHandler;           /* Our APM Event Handler */
    ULONG                   NotifyMask;             /* Mask of Events to be notified of */
    USHORT                  ClientDS;               /* Our DS Selector */
    USHORT                  DeviceID;               /* Devices to be notified of */
    UCHAR                   DevName[10];            /* 10 for dword alignment */
} APMNotDevice, *PAPMNotDevice;


// APM Event Identification (SubID )
#define APMEVENT_EnableAPM         3
#define APMEVENT_DisableAPM        4
#define APMEVENT_BIOS_Defauts      5
#define APMEVENT_SetPowerState     6
#define APMEVENT_BatteryLow        7
#define APMEVENT_NormalResume      8
#define APMEVENT_CriticalResume    9
#define APMEVENT_StandbyResume     10
#define APMEVENT_EngageAPM         11
#define APMEVENT_DisengageAPM      12
#define APMEVENT_PowerStatus       13
#define APMEVENT_UpdateTime        14
#define APMEVENT_OemEvent          15
#define APMEVENT_QueryPowerState   16
#define APMEVENT_CriticalSuspend   17
#define APMEVENT_EventNumHigh      17

// Notify defines (PwrState)

#define APMSTATE_READY           0x00
#define APMSTATE_STANDBY         0x01
#define APMSTATE_SUSPEND         0x02
#define APMSTATE_OFF             0x03
/*---------------------------------------------------------------------------*
 * Category 12 (Power Management) IOCtl Function Codes                       *
 *---------------------------------------------------------------------------*/

#define APM_Category           12    // Generic IOCtl Category for APM.

#define APM_SendEvent        0x40    // Function Codes.
#define APM_SetEventSem      0x41
#define APM_ConfirmEvent     0x42    // 0x42 is UNDOCUMENTED.
#define APM_BroadcastEvent   0x43    // 0x43 is UNDOCUMENTED.
#define APM_RegDaemonThread  0x44    // 0x44 is UNDOCUMENTED.
#define APM_OEMFunction      0x45
#define APM_SetResumeTimer   0x46
#define APM_QueryStatus      0x60
#define APM_QueryEvent       0x61
#define APM_QueryInfo        0x62
#define APM_QueryState       0x63
#define APM_QueryCaps        0x64
#define APM_QueryResumeTimer 0x65
#define APM_QuerySettings    0x66
//********************** Call function ******************************
// Semaphore, getting
typedef ULONG HAPPSEM;  // Application semaphore handle.

typedef struct _APM_SETEVENT_PPKT
{
    USHORT                  ReqFlags;     // Request flags.
    HAPPSEM                 hSem;         // Semaphore handle.
    ULONG                   NotifyMask;   // Notification Mask.
} APM_SETEVENT_PPKT, *PAPM_SETEVENT_PPKT;

/* Request flags bit definitions. */
#define  APM_SETEVENT_SETUP     0x0001    // Setup an Event Queue using
                                          // the semaphore submitted.
#define  APM_SETEVENT_RESET     0x0002    // Reset the semaphore.
#define  APM_SETEVENT_SEM16     0x0004    // 16-bit semaphore submitted.
#define  APM_SETEVENT_SEM32     0x0008    // 32-bit semaphore submitted.
#define  APM_SETEVENT_SEMNONE   0x0010    // No semaphore, update mask only.
#define  APM_SETEVENT_RESERVED  0xFFE0    // Reserved, future use.

#define  APM_SETEVENT_SEMTYPES  0x000C    // 16-bit and 32-bit semtypes only.
#define  APM_SETEVENT_SERVICES  0x0013    // Setup, reset, and semnone only.

/* Notification Mask Definitions. */
#define APMMASK_Idle              (0x00000001)
#define APMMASK_WakeUp            (0x00000002)
#define APMMASK_Busy              (0x00000004)
#define APMMASK_EnableAPM         (0x00000008)
#define APMMASK_DisableAPM        (0x00000010)
#define APMMASK_BIOS_Defaults     (0x00000020)
#define APMMASK_SetPowerState     (0x00000040)
#define APMMASK_BatteryLow        (0x00000080)
#define APMMASK_NormalResume      (0x00000100)
#define APMMASK_CriticalResume    (0x00000200)
#define APMMASK_StandbyResume     (0x00000400)
#define APMMASK_EngageAPM         (0x00000800)
#define APMMASK_DisengageAPM      (0x00001000)
#define APMMASK_PowerStatus       (0x00002000)
#define APMMASK_UpdateTime        (0x00004000)
#define APMMASK_OemEvent          (0x00008000)
#define APMMASK_QueryPowerState   (0x00010000)
#define APMMASK_CriticalSuspend   (0x00020000)
#define APMMASK_CapabilityChange  (0x00040000)
#define APMMASK_EnableRingResume  (0x00080000)
#define APMMASK_EnableTimerBased  (0x00100000)
#define APMMASK_ResumeTimerChange (0x00200000)
#define APMMASK_AllAppNotifyBits  (0x003ffff8)
/*---------------------------------------------------------------------------*
 * Function 0x61, Query Power Event                                          *
 *---------------------------------------------------------------------------*/

typedef struct _APM_QEVENT_PPKT            // Parameter Packet.
{

    USHORT                  ParmLength;   // Length, in bytes, of the Parm Packet.
    USHORT                  EventCount;   // Output:  number of events still queued when this call returns.
    USHORT                  Event;        // Output:  Power Event Code.
    USHORT                  reserved;
    USHORT                  DevId;        // Output:  Used only for SetPowerState event.
    USHORT                  PwrState;     // Output:  Used only for SetPowerState event.

} APM_QEVENT_PPKT, *PAPM_QEVENT_PPKT;

/* Power status information returned by a Normal Resume event.
 * (returned in the 'DevId' field of the APMGIO_QEVENT_PPKT structure)
 */
#define ResumeStat_PCMCIAOff   0x0001   // PCMCIA was powered off during suspend

/*---------------------------------------------------------------------------*
 * Function 0x64, Query Capabilities                                         *
 *---------------------------------------------------------------------------*/

typedef struct _APM_QCAPS_PPKT
{
    USHORT                  ParmLength;    // Length, in bytes, of the Parm Packet.
    USHORT                  BatteryMax;    // Output:  Max number of batteries supported by machine
    USHORT                  Capabilities;  // Output:  APM capabilities currently supported
} APM_QCAPS_PPKT, *PAPM_QCAPS_PPKT;

/* Capabilities bit flag fields.
 * When set, the associated capability is supported by the machine.
 */
#define  APM_CAPS_GlobalStandy       0x01   // Global standy
#define  APM_CAPS_GlobalSuspend      0x02   // Global suspend
#define  APM_CAPS_TimerStandbyWake   0x04   // Resume timer wakeup from standby
#define  APM_CAPS_TimerSuspendWake   0x08   // Resume timer wakeup from suspend
#define  APM_CAPS_SRingStandbyWake   0x10   // Serial ring indicator wakeup from standby
#define  APM_CAPS_SRingSuspendWake   0x20   // Serial ring indicator wakeup from suspend
#define  APM_CAPS_PRingStandbyWake   0x40   // PCMCIA ring indicator wakeup from standby
#define  APM_CAPS_PRingSuspendWake   0x80   // PCMCIA ring indicator wakeup from suspend
/*---------------------------------------------------------------------------*
 * Function 0x65, Query Resume Timer                                           *
 *---------------------------------------------------------------------------*/
// ???????????????????  "Not supported call 0x65 pLen:B dLen:2" Where 11 bytes structure ???????????
#ifdef Drova
typedef APM_RESUMETIMER_PPKT
  APM_QRESTIME_PPKT, *PAPM_QRESTIME_PPKT;
#endif

/*---------------------------------------------------------------------------*
 * Function 0x66, Query Power Settings
 *---------------------------------------------------------------------------*/

typedef struct _APM_QSETTINGS_PPKT
{
    USHORT                  ParmLength;    // Length, in bytes, of the Parm Packet.
    ULONG                   Settings;      // Output:  Current power management settings.
} APM_QSETTINGS_PPKT, *PAPM_QSETTINGS_PPKT;

/* APM Settings (APM_QSETTINGS_PPKT 'Settings' field flags) */
#define  APMSET_TimerBased     0x0001        // Timer based requests enabled
#define  APMSET_RingResume     0x0002        // Resume on ring indicator enabled

//******************************************************************************************
typedef struct _InternalAPM_
{
    ULONG                   VersionMajor;       // Version this APM
    ULONG                   VersionMinor;
} IntAPM, *PIntAPM;

typedef struct _RegDriversAPM_
{
    ULONG Count;          // The number of registered drivers
    ULONG BufferSize;
    void *Buffer;
} REGISTEREDDRIVERSAPM, *PREGISTEREDDRIVERSAPM;

typedef struct _APMInfo_
{
    USHORT                  CodeSeg;
    USHORT                  DataSeg;
    USHORT                  Offset;
    USHORT                  Flags;
    USHORT                  Level;
    ULONG                   IdleHook;          // 32offset for hook
    USHORT                  IdleHookSeg;       // Segment  for hook
} APMInfo, *PAPMInfo;

typedef struct _ACPI_Battery_
{
    ACPI_HANDLE             Handle;
// BIF
    ULONG                   PowerUnit;
    ULONG                   Capacity;
    ULONG                   LastFull;
    ULONG                   Technology;
    ULONG                   DesignVoltage;
    ULONG                   DesignCapacity;
    ULONG                   DesignWarning;
    ULONG                   DesignLow;
    ULONG                   LowToWarning;
    ULONG                   WarningToHigh;
// BST
    ULONG                   CurrentState;
    ULONG                   Rate;
    ULONG                   Remaining;
    ULONG                   CurrentVoltage;
} ACPIBattery, *PACPIBattery;

typedef struct _ApmWaitSetup_
{
    UINT32                  SuspendState;       // State can be ACPI_STATE_S1 ... S4
    APMIDCNotify            Notify;
    UINT32                  RetCode;
} APMWAITSETUP, *PAPMWAITSETUP;

typedef struct _SuspendSet_
{
    UCHAR                   SuspendState;                 // Which state set
    ULONG                   Resume;                       // Which use as resume
} SUSPENDSET, *PSUSPENDSET;

typedef struct _apmcfg_
{
    unsigned                State_HLT:         1;
    unsigned                State_C1:          1;
    unsigned                State_C2:          1;
    unsigned                State_C3:          1;
} APMCFG, *PAPMCFG;

#pragma pack()

// Error return codes.
#define APMERR_PowerNoError             0
#define APMERR_PowerBadSubId            1
#define APMERR_PowerBadReserved         2
#define APMERR_PowerBadDevId            3
#define APMERR_PowerBadPwrState         4
#define APMERR_PowerSemAlreadySetup     5
#define APMERR_PowerBadFlags            6
#define APMERR_PowerBadSemHandle        7
#define APMERR_PowerBadLength           8
#define APMERR_PowerDisabled            9
#define APMERR_PowerNoEventQueue       10
#define APMERR_PowerTooManyQueues      11
#define APMERR_PowerBiosError          12
#define APMERR_PowerBadSemaphore       13
#define APMERR_PowerQueueOverflow      14
#define APMERR_PowerStateChangeReject  15
#define APMERR_PowerNotSupported       16
#define APMERR_PowerDisengaged         17
#define APMERR_PowerNoCapability       18
#define APMERR_PowerBadTime            19
#define APMERR_PowerBadFunctionCode    20
#define APMERR_PowerHighestErrCode     20

/* APM IDC Function Codes. */
#define APMIDC_Register          0
#define APMIDC_Deregister        1
#define APMIDC_SendEvent         2
#define APMIDC_QueryStatus       3
#define APMIDC_QueryInfo         4
#define APMIDC_QueryDaemon       5
#define APMIDC_QueryState        6
#define APMIDC_OEMFunction       7
#define APMIDC_SetResumeTimer    8
#define APMIDC_QueryCaps         9
#define APMIDC_QueryResumeTimer 10
#define APMIDC_QuerySettings    11

// APM IDC Error Codes.
#define APMIDC_InvalidFunction 128
#define APMIDC_InvalidHandle   129
#define APMIDC_TooManyClients  130
#define APMIDC_Busy            131

ULONG GetACStatus(void);
ULONG GetBST(ULONG Number);
void GetPowerStatus(QueryPowerStatus *cur,ULONG size);
void AcpiC1StateHook(void);
void AcpiC2StateHook(void);
void AcpiC3StateHook(void);

// Category
#define ACPIAPM_CALL                  0x88
// Functions
#define ACPIAPM_INTERNALINFO          0x81
#define ACPIAPM_INTERNALTHREAD        0x82
#define ACPIAPM_SENDEVENT             0x83
#define ACPIAPM_SENDSEMAPHORE         0x84
#define ACPIAPM_SETPARAMETERS         0x85
#define ACPIAPM_SETBATTERY            0x86
#define ACPIAPM_GETREGISTERED         0x87
