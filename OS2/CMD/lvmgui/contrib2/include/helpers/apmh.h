
/*
 *@@sourcefile apmh.h:
 *      header file for apmh.c.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #include <os2.h>
 *@@include #include "helpers\apmh.h"
 */

/*      Copyright (C) 1998-2001 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef APMH_HEADER_INCLUDED
    #define APMH_HEADER_INCLUDED

    /* ******************************************************************
     *
     *   APM definitions (from DDK)
     *
     ********************************************************************/

    /*---------------------------------------------------------------------------*
     * Category 12 (Power Management) IOCtl Function Codes                       *
     *---------------------------------------------------------------------------*/

    #define APMGIO_Category           12    // Generic IOCtl Category for APM.

    #define APMGIO_SendEvent        0x40    // Function Codes.
    #define APMGIO_SetEventSem      0x41
    #define APMGIO_ConfirmEvent     0x42    // 0x42 is UNDOCUMENTED.
    #define APMGIO_BroadcastEvent   0x43    // 0x43 is UNDOCUMENTED.
    #define APMGIO_RegDaemonThread  0x44    // 0x44 is UNDOCUMENTED.
    #define APMGIO_OEMFunction      0x45
    #define APMGIO_QueryStatus      0x60
    #define APMGIO_QueryEvent       0x61
    #define APMGIO_QueryInfo        0x62
    #define APMGIO_QueryState       0x63

    #pragma pack(1)

    /*---------------------------------------------------------------------------*
     * Function 0x60, Query Power Status                                         *
     * Reference MS/Intel APM specification for interpretation of status codes.  *
     *---------------------------------------------------------------------------*/

    typedef struct _APMGIO_QSTATUS_PPKT // Parameter Packet.
    {
      USHORT ParmLength;    // Length, in bytes, of the Parm Packet.
      USHORT Flags;         // Output:  Flags.
      UCHAR  ACStatus;
                            // Output:  0x00 if not on AC,
                            //          0x01 if AC,
                            //          0x02 if on backup power,
                            //          0xFF if unknown
      UCHAR  BatteryStatus;
                            // Output:  0x00 if battery high,
                            //          0x01 if battery low,
                            //          0x02 if battery critically low,
                            //          0x03 if battery charging
                            //          0xFF if unknown
      UCHAR  BatteryLife;
                            // Output:  Battery power life (as percentage)
      UCHAR  BatteryTimeForm;
                            // Output:
                            //          0x00 if format is seconds,
                            //          0x01 if format is minutes,
                            //          0xFF if unknown
      USHORT BatteryTime;   // Output:  Remaining battery time.
      UCHAR  BatteryFlags;  // Output:  Battery status flags

    } APMGIO_QSTATUS_PPKT, *NPAPMGIO_QSTATUS_PPKT, FAR *PAPMGIO_QSTATUS_PPKT;

    typedef struct _APMGIO_10_QSTATUS_PPKT {        // Parameter Packet for
                                                    // APM 1.0 interface
      USHORT ParmLength;    // Length, in bytes, of the Parm Packet.
      USHORT Flags;         // Output:  Flags.
      UCHAR  ACStatus;      // Output:  AC line power status.
      UCHAR  BatteryStatus; // Output:  Battery power status
      UCHAR  BatteryLife;   // Output:  Battery power status

    } APMGIO_10_QSTATUS_PPKT;

    // Error return codes.
    #define GIOERR_PowerNoError             0
    #define GIOERR_PowerBadSubId            1
    #define GIOERR_PowerBadReserved         2
    #define GIOERR_PowerBadDevId            3
    #define GIOERR_PowerBadPwrState         4
    #define GIOERR_PowerSemAlreadySetup     5
    #define GIOERR_PowerBadFlags            6
    #define GIOERR_PowerBadSemHandle        7
    #define GIOERR_PowerBadLength           8
    #define GIOERR_PowerDisabled            9
    #define GIOERR_PowerNoEventQueue       10
    #define GIOERR_PowerTooManyQueues      11
    #define GIOERR_PowerBiosError          12
    #define GIOERR_PowerBadSemaphore       13
    #define GIOERR_PowerQueueOverflow      14
    #define GIOERR_PowerStateChangeReject  15
    #define GIOERR_PowerNotSupported       16
    #define GIOERR_PowerDisengaged         17
    #define GIOERR_PowerHighestErrCode     17

    typedef struct _APMGIO_DPKT {

      USHORT ReturnCode;

    } APMGIO_DPKT, *NPAPMGIO_DPKT, FAR *PAPMGIO_DPKT;

    /*
     * define constants and type for APM function
     */

    #define POWER_SUBID_ENABLE_APM                  0x0003
    #define POWER_SUBID_SET_POWER_STATE             0x0006

    #define POWER_DEVID_ALL_DEVICES                 0x0001

    #define POWER_STATE_OFF                         0x0003

    #define BIOSFLAG_APMSUPPORTED                   0x0008

    typedef struct
    {
        USHORT  usSubID;
        USHORT  usReserved;
        USHORT  usData1;
        USHORT  usData2;
    } SENDPOWEREVENT;

    typedef struct
    {
        USHORT  usParmLength;
        USHORT  usBIOSFlags;
        USHORT  usBIOSVersion;
        USHORT  usDriverVersion;
    } GETPOWERINFO;

    #pragma pack()

    /* ******************************************************************
     *
     *   APM helper APIs
     *
     ********************************************************************/

    #pragma pack(1)

    /*
     *@@ APM:
     *      APM monitor data returned from apmhOpen.
     *
     *@@added V0.9.14 (2001-08-01) [umoeller]
     *@@changed V0.9.19 (2002-05-28) [umoeller]: added fUsingAC
     */

    typedef struct _APM
    {
        HFILE   hfAPMSys;

        USHORT  usBIOSVersion,
                usDriverVersion,
                usLowestAPMVersion;

        BYTE    fAlreadyRead;
                    // TRUE after the first call to aphReadStatus

        // the following are valid after a call to apmhReadStatus

        // Note: According to Eirik, his laptop did not return
        // bBatteryStatus == 0x03 after the battery had finished
        // charging. So bBatteryStatus cannot be used to determine
        // whether the system is running on AC or not. Hence the
        // addition of fUsingAC.
        // V0.9.19 (2002-05-28) [umoeller]
        BYTE    fUsingAC;
                            // 0x00 if not on AC,
                            // 0x01 if AC,
                            // 0x02 if on backup power,
                            // 0xFF if unknown

        BYTE    bBatteryStatus;
                    // copy of APM battery status, that is:
                            // Output:  0x00 if battery high,
                            //          0x01 if battery low,
                            //          0x02 if battery critically low,
                            //          0x03 if battery charging
                            //          0xFF if unknown
        BYTE    bBatteryLife;
                    // current battery life as percentage (0-100)

    } APM, *PAPM;

    #pragma pack()

    APIRET APIENTRY apmhIOCtl(HFILE hfAPMSys, ULONG ulFunction, PVOID pvParamPck, ULONG cbParamPck);
    typedef APIRET APIENTRY APMHIOCTL(HFILE hfAPMSys, ULONG ulFunction, PVOID pvParamPck, ULONG cbParamPck);
    typedef APMHIOCTL *PAPMHIOCTL;

    APIRET APIENTRY apmhOpen(PAPM *ppApm);
    typedef APIRET APIENTRY APMHOPEN(PAPM *ppApm);
    typedef APMHOPEN *PAPMHOPEN;

    APIRET APIENTRY apmhReadStatus(PAPM pApm, PBOOL pfChanged);
    typedef APIRET APIENTRY APMHREADSTATUS(PAPM pApm, PBOOL pfChanged);
    typedef APMHREADSTATUS *PAPMHREADSTATUS;

    VOID APIENTRY apmhClose(PAPM *ppApm);
    typedef VOID APIENTRY APMHCLOSE(PAPM *ppApm);
    typedef APMHCLOSE *PAPMHCLOSE;

    BOOL apmhHasBattery(VOID);

#endif

#if __cplusplus
}
#endif

