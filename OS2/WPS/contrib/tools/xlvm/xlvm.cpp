
/*
 *@@sourcefile xlvm.cpp:
 *
 *
 *@@header "xlvm.h"
 *@@added V0.9.9 (2001-02-28) [umoeller]
 */

/*
 *      Copyright (C) 2001-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WIN
#define INCL_GPIPRIMITIVES
#include <os2.h>

#include "lvm_intr.h"

#include "setup.h"
#include "dlgids.h"                     // for buildlevel

#include "helpers\cnrh.h"
#include "helpers\comctl.h"
#include "helpers\datetime.h"           // date/time helper routines
#include "helpers\dialog.h"
#include "helpers\dosh.h"
#include "helpers\except.h"
#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\nls.h"
#include "helpers\prfh.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\threads.h"
#include "helpers\tree.h"
#include "helpers\winh.h"
#include "helpers\xstring.h"

#include "xlvm.h"

#pragma info (nocnv)

/* ******************************************************************
 *
 *   Private declarations
 *
 ********************************************************************/

/*
 *@@ LVMDRIVEDATA:
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 */

typedef struct _LVMDRIVEDATA
{
    HWND                        hwndChart,          // chart for this drive
                                hwndDescription;    // static below

    ADDRESS                     DriveHandle;

    Drive_Control_Record        *pControlRecord;
                // do not free this, this points to an item in the
                // DriveData array
    /*
    typedef struct _Drive_Control_Record {
    CARDINAL32   Drive_Number;                   // OS/2 Drive Number for this drive.
    CARDINAL32   Drive_Size;                     // The total number of sectors on the drive.
    DoubleWord   Drive_Serial_Number;            // The serial number assigned to this drive.  For info. purposes only.
    ADDRESS      Drive_Handle;                   // Handle used for operations on the disk that this record corresponds to.
    CARDINAL32   Cylinder_Count;                 // The number of cylinders on the drive.
    CARDINAL32   Heads_Per_Cylinder;             // The number of heads per cylinder for this drive.
    CARDINAL32   Sectors_Per_Track;              // The number of sectors per track for this drive.
    BOOLEAN      Drive_Is_PRM;                   // Set to TRUE if this drive is a PRM.
    BYTE         Reserved[3];                    // Alignment.
    } Drive_Control_Record;
    */

    Drive_Information_Record    InfoRecord;
                // full struct, no pointer

    /*
    typedef struct _Drive_Information_Record {
    CARDINAL32   Total_Available_Sectors;        // The number of sectors on the disk which are not currently assigned to a partition.
    CARDINAL32   Largest_Free_Block_Of_Sectors;  // The number of sectors in the largest contiguous block of available sectors.
    BOOLEAN      Corrupt_Partition_Table;        // If TRUE, then the partitioning information found on the drive is incorrect!
    BOOLEAN      Unusable;                       // If TRUE, the drive's MBR is not accessible and the drive can not be partitioned.
    BOOLEAN      IO_Error;                       // If TRUE, then the last I/O operation on this drive failed!
    BOOLEAN      Is_Big_Floppy;                  // If TRUE, then the drive is a PRM formatted as a big floppy (i.e. the old style removable media support).
    char         Drive_Name[DISK_NAME_SIZE];     // User assigned name for this disk drive.
    } Drive_Information_Record;
    */

    Partition_Information_Array PartitionsArray;
                // member array pointer is freed for each node
                // in FreeLVMData

    /* typedef struct _Partition_Information_Array {
    Partition_Information_Record * Partition_Array; // An array of Partition_Information_Records.
    CARDINAL32                     Count;           // The number of entries in the Partition_Array.
    } Partition_Information_Array; */

} LVMDRIVEDATA, *PLVMDRIVEDATA;

/*
 *@@ LVMDATA:
 *      structure holding the current LVM information.
 *
 *      Allocated and filled once; refreshed after
 *      the LVM setup has been changed.
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 */

typedef struct _LVMDATA
{
    HWND                    hwndClient;

    BOOL                    fLVMOpened;

    BOOL                    fDirty;
                    // if this becomes TRUE, changes have been made and the
                    // user should be prompted for whether those should be
                    // committed

    Drive_Control_Array     DriveData;
                    // opened by Get_Drive_Control_Data, is freed in
                    // FreeLVMData

    LINKLIST                llDriveData;
                    // linked list of drive data;
                    // points to LVMDRIVEDATA structs
                    // (no auto-free, must each be freed)

    LINKLIST                llPartitions;
                    // linked list to all partitions in the LVMDRIVEDATA structs;
                    // must not be freed

    Partition_Information_Record *pPartitionSource;
                    // partition which currently has source emphasis or NULL if none

} LVMDATA, *PLVMDATA;

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

FILE        *G_LogFile = NULL;

static const PCSZ G_pcszDefaultFont = "9.WarpSans";

HWND        G_hwndStatusBar = NULLHANDLE;

// context menus
HWND        G_hwndWhitespaceMenu = NULLHANDLE,
            G_hwndFreeSpaceMenu = NULLHANDLE,
            G_hwndPartitionMenu = NULLHANDLE,
            G_hwndVolumeMenu = NULLHANDLE;

ULONG       G_ulViewStyle = IDMI_VIEW_BARCHART;

HWND        G_hwndMenuShowing = NULLHANDLE; // only while menu is showing
EMPHASISNOTIFY G_CurrentEmphasis;

PFNWP       G_fnwpFrameOrig = NULL;

HPOINTER    G_hptrDrive = NULLHANDLE,
            G_hptrPartition = NULLHANDLE;

const char  *INIAPP              = "XWorkplace:xlvm";
const char  *INIKEY_MAINWINPOS   = "WinPos";
const char  *INIKEY_OPENDLG      = "FileOpenDlgPath";

const char  *APPTITLE            = "xlvm";

#define XM_INSERTDATA       WM_USER
#define XM_LVMERROR         (WM_USER + 1)
#define XM_ENABLEITEMS      (WM_USER + 2)

#define MB_FROM_SECTORS(ulSize) ((((ulSize) / 2) + 512) / 1024)

static const CONTROLDEF
        G_OKButton = CONTROLDEF_DEFPUSHBUTTON(
                        "~OK",
                        DID_OK,
                        STD_BUTTON_WIDTH,
                        STD_BUTTON_HEIGHT),
        G_CancelButton = CONTROLDEF_PUSHBUTTON(
                        "~Cancel",
                        DID_CANCEL,
                        STD_BUTTON_WIDTH,
                        STD_BUTTON_HEIGHT);

// LVM data

COUNTRYSETTINGS2        G_CountrySettings;

/* ******************************************************************
 *
 *   Load LVM Data
 *
 ********************************************************************/

/*
 *@@ LVMError:
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

VOID LVMError(HWND hwndClient,
              ULONG ulrc,
              const char *pcszContext)
{
    fprintf(G_LogFile,
            "LVMError: %s returned %d\n",
            pcszContext,
            ulrc);
    WinPostMsg(hwndClient,
               XM_LVMERROR,
               (MPARAM)ulrc,
               (MPARAM)pcszContext);
}

PCSZ DescribeLVMError(ULONG ulrc)
{
    switch (ulrc)
    {
        case LVM_ENGINE_NO_ERROR: return "NO_ERROR";
        case LVM_ENGINE_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
        case LVM_ENGINE_IO_ERROR: return "IO_ERROR";
        case LVM_ENGINE_BAD_HANDLE: return "BAD_HANDLE";
        case LVM_ENGINE_INTERNAL_ERROR: return "INTERNAL_ERROR";
        case LVM_ENGINE_ALREADY_OPEN: return "ALREADY_OPEN";
        case LVM_ENGINE_NOT_OPEN: return "NOT_OPEN";
        case LVM_ENGINE_NAME_TOO_BIG: return "NAME_TOO_BIG";
        case LVM_ENGINE_OPERATION_NOT_ALLOWED: return "OPERATION_NOT_ALLOWED";
        case LVM_ENGINE_DRIVE_OPEN_FAILURE: return "DRIVE_OPEN_FAILURE";
        case LVM_ENGINE_BAD_PARTITION: return "BAD_PARTITION";
        case LVM_ENGINE_CAN_NOT_MAKE_PRIMARY_PARTITION: return "CAN_NOT_MAKE_PRIMARY_PARTITION";
        case LVM_ENGINE_TOO_MANY_PRIMARY_PARTITIONS: return "TOO_MANY_PRIMARY_PARTITIONS";
        case LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE: return "CAN_NOT_MAKE_LOGICAL_DRIVE";
        case LVM_ENGINE_REQUESTED_SIZE_TOO_BIG: return "REQUESTED_SIZE_TOO_BIG";
        case LVM_ENGINE_1024_CYLINDER_LIMIT: return "1024_CYLINDER_LIMIT";
        case LVM_ENGINE_PARTITION_ALIGNMENT_ERROR: return "PARTITION_ALIGNMENT_ERROR";
        case LVM_ENGINE_REQUESTED_SIZE_TOO_SMALL: return "REQUESTED_SIZE_TOO_SMALL";
        case LVM_ENGINE_NOT_ENOUGH_FREE_SPACE: return "NOT_ENOUGH_FREE_SPACE";
        case LVM_ENGINE_BAD_ALLOCATION_ALGORITHM: return "BAD_ALLOCATION_ALGORITHM";
        case LVM_ENGINE_DUPLICATE_NAME: return "DUPLICATE_NAME";
        case LVM_ENGINE_BAD_NAME: return "BAD_NAME";
        case LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE: return "BAD_DRIVE_LETTER_PREFERENCE";
        case LVM_ENGINE_NO_DRIVES_FOUND: return "NO_DRIVES_FOUND";
        case LVM_ENGINE_WRONG_VOLUME_TYPE: return "WRONG_VOLUME_TYPE";
        case LVM_ENGINE_VOLUME_TOO_SMALL: return "VOLUME_TOO_SMALL";
        case LVM_ENGINE_BOOT_MANAGER_ALREADY_INSTALLED: return "BOOT_MANAGER_ALREADY_INSTALLED";
        case LVM_ENGINE_BOOT_MANAGER_NOT_FOUND: return "BOOT_MANAGER_NOT_FOUND";
        case LVM_ENGINE_INVALID_PARAMETER: return "INVALID_PARAMETER";
        case LVM_ENGINE_BAD_FEATURE_SET: return "BAD_FEATURE_SET";
        case LVM_ENGINE_TOO_MANY_PARTITIONS_SPECIFIED: return "TOO_MANY_PARTITIONS_SPECIFIED";
        case LVM_ENGINE_LVM_PARTITIONS_NOT_BOOTABLE: return "LVM_PARTITIONS_NOT_BOOTABLE";
        case LVM_ENGINE_PARTITION_ALREADY_IN_USE: return "PARTITION_ALREADY_IN_USE";
        case LVM_ENGINE_SELECTED_PARTITION_NOT_BOOTABLE: return "SELECTED_PARTITION_NOT_BOOTABLE";
        case LVM_ENGINE_VOLUME_NOT_FOUND: return "VOLUME_NOT_FOUND";
        case LVM_ENGINE_DRIVE_NOT_FOUND: return "DRIVE_NOT_FOUND";
        case LVM_ENGINE_PARTITION_NOT_FOUND: return "PARTITION_NOT_FOUND";
        case LVM_ENGINE_TOO_MANY_FEATURES_ACTIVE: return "TOO_MANY_FEATURES_ACTIVE";
        case LVM_ENGINE_PARTITION_TOO_SMALL: return "PARTITION_TOO_SMALL";
        case LVM_ENGINE_MAX_PARTITIONS_ALREADY_IN_USE: return "MAX_PARTITIONS_ALREADY_IN_USE";
        case LVM_ENGINE_IO_REQUEST_OUT_OF_RANGE: return "IO_REQUEST_OUT_OF_RANGE";
        case LVM_ENGINE_SPECIFIED_PARTITION_NOT_STARTABLE: return "SPECIFIED_PARTITION_NOT_STARTABLE";
        case LVM_ENGINE_SELECTED_VOLUME_NOT_STARTABLE: return "SELECTED_VOLUME_NOT_STARTABLE";
        case LVM_ENGINE_EXTENDFS_FAILED: return "EXTENDFS_FAILED";
        case LVM_ENGINE_REBOOT_REQUIRED: return "REBOOT_REQUIRED";
        case LVM_ENGINE_CAN_NOT_OPEN_LOG_FILE: return "CAN_NOT_OPEN_LOG_FILE";
        case LVM_ENGINE_CAN_NOT_WRITE_TO_LOG_FILE: return "CAN_NOT_WRITE_TO_LOG_FILE";
        case LVM_ENGINE_REDISCOVER_FAILED: return "REDISCOVER_FAILED";
        case LVM_ENGINE_INTERNAL_VERSION_FAILURE: return "INTERNAL_VERSION_FAILURE";
        case LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE: return "PLUGIN_OPERATION_INCOMPLETE";
        case LVM_ENGINE_BAD_FEATURE_ID: return "BAD_FEATURE_ID";
        case LVM_ENGINE_NO_INIT_DATA: return "NO_INIT_DATA";
        case LVM_ENGINE_NO_CONTEXT_DATA: return "NO_CONTEXT_DATA";
        case LVM_ENGINE_WRONG_CLASS_FOR_FEATURE: return "WRONG_CLASS_FOR_FEATURE";
        case LVM_ENGINE_INCOMPATIBLE_FEATURES_SELECTED: return "INCOMPATIBLE_FEATURES_SELECTED";
        case LVM_ENGINE_NO_CHILDREN: return "NO_CHILDREN";
        case LVM_ENGINE_FEATURE_NOT_SUPPORTED_BY_INTERFACE: return "FEATURE_NOT_SUPPORTED_BY_INTERFACE";
        case LVM_ENGINE_NO_PARENT: return "NO_PARENT";
        case LVM_ENGINE_VOLUME_HAS_NOT_BEEN_COMMITTED_YET: return "VOLUME_HAS_NOT_BEEN_COMMITTED_YET";
        case LVM_ENGINE_UNABLE_TO_REFERENCE_VOLUME: return "UNABLE_TO_REFERENCE_VOLUME";
        case LVM_ENGINE_PARSING_ERROR: return "PARSING_ERROR";
        case LVM_ENGINE_INTERNAL_FEATURE_ERROR: return "INTERNAL_FEATURE_ERROR";
        case LVM_ENGINE_VOLUME_NOT_CONVERTED: return "VOLUME_NOT_CONVERTED";
    }

    return "unknown code";
}

/*
 *@@ fntLoadLVMData:
 *      fills an LVMDATA struct with the LVM data so that
 *      we can retrieve them more quickly.
 *
 *      THREADINFO.ulData must be a pointer to the
 *      LVMDATA to fill.
 *
 *@@added V0.9.9 (2001-02-28) [umoeller]
 */

VOID _Optlink fntLoadLVMData(PTHREADINFO ptiMyself)
{
    CARDINAL32 Error = 0;

    TRY_LOUD(excpt1)
    {
        PLVMDATA pData = (PLVMDATA)ptiMyself->ulData;

        fprintf(G_LogFile, __FUNCTION__ ": entering\n");

        if (!pData->fLVMOpened)
        {
            // first call: open LVM
            Open_LVM_Engine(FALSE,
                            &Error);

            if (Error)
                LVMError(pData->hwndClient, Error, "Open_LVM_Engine");
            else
                pData->fLVMOpened = TRUE;
        }

        if (pData->fLVMOpened)
        {
            pData->DriveData = Get_Drive_Control_Data(&Error);

            if (Error)
                LVMError(pData->hwndClient, Error, "Get_Drive_Control_Data");
            else
            {
                // go through drives
                BOOL fExit = FALSE;
                ULONG ul;
                for (ul = 0;
                     (ul < pData->DriveData.Count) && (!fExit);
                     ul++)
                {
                    PLVMDRIVEDATA pDriveData = NEW(LVMDRIVEDATA);
                    ZERO(pDriveData);

                    pDriveData->pControlRecord = &pData->DriveData.Drive_Control_Data[ul];
                    // copy handle for quick access
                    pDriveData->DriveHandle = pDriveData->pControlRecord->Drive_Handle;
                    pDriveData->InfoRecord = Get_Drive_Status(pDriveData->DriveHandle,
                                                              &Error);

                    if (Error)
                    {
                        LVMError(pData->hwndClient, Error, "Get_Drive_Status");
                        fExit = TRUE;
                    }
                    else
                    {
                        // now get the partitions for this drive
                        pDriveData->PartitionsArray = Get_Partitions(pDriveData->DriveHandle,
                                                                     &Error);

                        if (Error)
                        {
                            LVMError(pData->hwndClient, Error, "Get_Partitions");
                            fExit = TRUE;
                        }
                        else
                        {
                            // go thru partitions
                            ULONG ul2;
                            for (ul2 = 0;
                                 ul2 < pDriveData->PartitionsArray.Count;
                                 ul2++)
                            {
                                Partition_Information_Record *pBarItem
                                    = &pDriveData->PartitionsArray.Partition_Array[ul2];

                                // store partition in global list of all partitions
                                lstAppendItem(&pData->llPartitions,
                                              pBarItem);
                            }

                            // store drive in global list
                            lstAppendItem(&pData->llDriveData,
                                          pDriveData);
                        }
                    }
                }
            }
        }

        fprintf(G_LogFile, __FUNCTION__ ": leaving\n"
                   );
    }
    CATCH(excpt1)
    {

    } END_CATCH();

    WinPostMsg(ptiMyself->hwndNotify,
               WM_USER,
               (MPARAM)Error,
               0);
}

/*
 *@@ FreeLVMData:
 *      frees allocated memory.
 *
 *      If (fCloseEngine == TRUE), the engine is closed
 *      unconditionally, and all changes are lost.
 *
 *      Called on WM_CLOSE with fCloseEngine == TRUE.
 *      Called during program execution if the display
 *      needs to be refreshed.
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 */

VOID FreeLVMData(PLVMDATA pData,
                 BOOL fCloseEngine)
{
    // clean up
    if (pData->DriveData.Drive_Control_Data)
        Free_Engine_Memory(pData->DriveData.Drive_Control_Data);

    // clear list of partition pointers into drive data
    PLISTNODE pNode = lstQueryFirstNode(&pData->llDriveData);
    while (pNode)
    {
        PLVMDRIVEDATA pDriveData = (PLVMDRIVEDATA)pNode->pItemData;

        if (pDriveData->hwndChart)
        {
            WinDestroyWindow(pDriveData->hwndChart);
            pDriveData->hwndChart = NULLHANDLE;
        }
        if (pDriveData->hwndDescription)
        {
            WinDestroyWindow(pDriveData->hwndDescription);
            pDriveData->hwndDescription = NULLHANDLE;
        }

        if (pDriveData->PartitionsArray.Partition_Array)
            Free_Engine_Memory(pDriveData->PartitionsArray.Partition_Array);

        free(pDriveData);

        pNode = pNode->pNext;
    }

    lstClear(&pData->llDriveData);

    // clear list of partition pointers into drive data too
    lstClear(&pData->llPartitions);

    if (    (pData->fLVMOpened)
         && (fCloseEngine)
       )
    {
        // close LVM
        Close_LVM_Engine();

        pData->fLVMOpened = FALSE;
        pData->fDirty = FALSE;
    }
}

/* ******************************************************************
 *
 *   Helpers
 *
 ********************************************************************/

/*
 *@@ MessageBox:
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

ULONG MessageBox(HWND hwndOwner,
                 XSTRING *pstr,
                 ULONG fl)
{
    static MSGBOXSTRINGS MsgBoxStrings =
        {
            "~Yes",
            "~No",
            "~OK",
            "~Cancel",
            "~Abort",
            "~Retry",
            "~Ignore",
            "~Enter",
            "Yes to ~all"
        };

    return dlghMessageBox(hwndOwner,
                          G_hptrDrive,
                          APPTITLE,
                          pstr->psz,
                          NULL,
                          fl,
                          G_pcszDefaultFont,
                          &MsgBoxStrings);

    /* return (WinMessageBox(HWND_DESKTOP,
                          G_hwndMain,
                          pcszText,
                          APPTITLE,
                          0,
                          fl | MB_MOVEABLE)); */
}

VOID NotImplemented(HWND hwndOwner)
{
    XSTRING str;
    xstrInitCopy(&str, "Not implemented yet.", 0);
    MessageBox(hwndOwner, &str, MB_CANCEL);
    xstrClear(&str);
}

/*
 *@@ SetChartStyle:
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

VOID SetChartStyle(HWND hwndChart)
{
    CHARTSTYLE cs;
    if (G_ulViewStyle == IDMI_VIEW_BARCHART)
        // bar chart:
        cs.ulStyle =    CHS_BARCHART
                      | CHS_3D_DARKEN
                      | CHS_DESCRIPTIONS
                      | CHS_DRAWLINES
                      | CHS_SELECTIONS
                      ;
    else
        // pie chart:
        cs.ulStyle =    CHS_PIECHART
                      | CHS_3D_DARKEN
                      | CHS_DESCRIPTIONS
                      | CHS_DRAWLINES
                      | CHS_SELECTIONS
                      ;

    cs.ulThickness = 20;
    cs.dPieSize = .8;
    cs.dDescriptions = .8;
    WinSendMsg(hwndChart, CHTM_SETCHARTSTYLE, &cs, NULL);
}

/*
 *@@ RefreshViewMenu:
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

VOID RefreshViewMenu(HWND hwndMain)
{
    HWND hwndMenu;
    if (hwndMenu = WinWindowFromID(hwndMain, FID_MENU))
    {
        WinCheckMenuItem(hwndMenu,
                         IDMI_VIEW_BARCHART,
                         (G_ulViewStyle == IDMI_VIEW_BARCHART));
        WinCheckMenuItem(hwndMenu,
                         IDMI_VIEW_PIECHART,
                         (G_ulViewStyle == IDMI_VIEW_PIECHART));
    }
}

/* ******************************************************************
 *
 *   Command handlers
 *
 ********************************************************************/

#define ID_CREATE_TYPEGROUP                 1100
#define ID_CREATE_PRIMARY_RADIO             1101
#define ID_CREATE_LOGICALBOOTABLE_RADIO     1102
#define ID_CREATE_LVM_RADIO                 1103
#define ID_CREATE_COMPAT_RADIO              1104

#define ID_CREATE_SIZETXT                   1111
#define ID_CREATE_SIZESLIDER                1112
#define ID_CREATE_SIZEENTRYFIELD            1113
#define ID_CREATE_SIZEMB                    1114

#define ID_CREATE_NAMETXT                   1121
#define ID_CREATE_NAMEEF                    1122

#define ID_CREATE_LETTERTXT                 1131
#define ID_CREATE_LETTERDROP                1132

/*
 *@@ fnwpConfirmCreateDlg:
 *
 *@@added V0.9.14 (2001-08-03) [umoeller]
 */

MRESULT EXPENTRY fnwpConfirmCreateDlg(HWND hwndDlg, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    PLVMDATA pData = (PLVMDATA)WinQueryWindowPtr(hwndDlg, QWL_USER);

    switch (msg)
    {
        case WM_INITDLG:
            WinSetWindowPtr(hwndDlg, QWL_USER, mp2);
        break;

#if 1
        case WM_CONTROL:
            switch (SHORT1FROMMP(mp1))
            {
                case ID_CREATE_SIZESLIDER:
                {
                    LONG lSliderIndex = winhQuerySliderArmPosition(WinWindowFromID(hwndDlg, ID_CREATE_SIZESLIDER),
                                                                   SMA_INCREMENTVALUE);
                            // 0 - 99
                    LONG    lPercent = lSliderIndex + 1;

                    CHAR    szMaxSize[100],
                            szMaxTxt[100];

                    LONG    lMBCurrent =   (double)MB_FROM_SECTORS(pData->pPartitionSource->Usable_Partition_Size)
                                         * lPercent
                                         / 100;

                    sprintf(szMaxTxt,
                            "%d",
                            lMBCurrent);
                    WinSetDlgItemText(hwndDlg, ID_CREATE_SIZEENTRYFIELD, szMaxTxt);
                }
                break;
            }
        break;

#else
        case WM_CONTROL:
        {
            SHORT usid = SHORT1FROMMP(mp1),
                  uscode = SHORT2FROMMP(mp1);
            switch (usid)
            {
                case IDDI_CREATE_PARTITIONSIZE_SPIN:
                    winhAdjustDlgItemSpinData(hwndDlg,
                                              IDDI_CREATE_PARTITIONSIZE_SPIN,
                                              // jump in steps of 1 MB == 2048 sectors
                                              -2048,
                                              uscode);
                    WinSendMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
                break;

                case IDDI_CREATE_PARTITION_CHBX:
                case IDDI_CREATE_VOLUME_CHBX:
                    WinSendMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);
                break;
            }
        }
        break;

        case XM_ENABLEITEMS:
        {
            CHAR    szMB[100],
                    szTemp[100];

            LONG lSectors = winhAdjustDlgItemSpinData(hwndDlg,
                                                      IDDI_CREATE_PARTITIONSIZE_SPIN,
                                                      0,        // no grid
                                                      0);
            sprintf(szMB,
                    "%s MB",
                    nlsThousandsULong(szTemp,
                                      MB_FROM_SECTORS(lSectors),
                                      G_CountrySettings.cs.cThousands));

            WinSetDlgItemText(hwndDlg, IDDI_CREATE_PARTITIONSIZE_MB, szMB);

            BOOL fEnable = winhIsDlgItemChecked(hwndDlg, IDDI_CREATE_PARTITION_CHBX);
            USHORT ausPartitionIDs[]
                = {
                        IDDI_CREATE_PARTITIONSIZE_TXT,
                        // IDDI_CREATE_PARTITION_MAINGRP,
                        IDDI_CREATE_PARTITIONSIZE_SPIN,
                        IDDI_CREATE_PARTITIONSIZE_MB,
                        IDDI_CREATE_PARTITIONNAME_TXT,
                        IDDI_CREATE_PARTITIONNAME_EF,
                        IDDI_CREATE_PARTITIONTYPE_GRP,
                        IDDI_CREATE_PARTITION_PRIMARY_CHBX,
                        IDDI_CREATE_PARTITION_LOGICAL_CHBX,
                        IDDI_CREATE_PARTITIONALLOC_GRP,
                        IDDI_CREATE_PARTITION_ALLOCSTART_CHBX,
                        IDDI_CREATE_PARTITION_ALLOCEND_CHBX,
                  };

            ULONG ul = 0;

            for (;
                 ul < ARRAYITEMCOUNT(ausPartitionIDs);
                 ul++)
                WinEnableControl(hwndDlg, ausPartitionIDs[ul], fEnable);

            // enable volume if create partition is checked
            // or if we have a partition already
            BOOL fCanCreateVolume = (    (fEnable)
                                      || (pData->pPartitionSource->Partition_Type)
                                    ),
                 fVolumeChecked = FALSE;

            WinEnableControl(hwndDlg, IDDI_CREATE_VOLUME_CHBX,
                             fCanCreateVolume);
            if (!fCanCreateVolume)
                winhSetDlgItemChecked(hwndDlg, IDDI_CREATE_VOLUME_CHBX, FALSE);
            else
                fVolumeChecked = !!winhIsDlgItemChecked(hwndDlg, IDDI_CREATE_VOLUME_CHBX);

            USHORT ausVolumeIDs[]
                = {
                        IDDI_CREATE_VOLUMENAME_TXT,
                        IDDI_CREATE_VOLUMENAME_EF
                  };
            for (ul = 0;
                 ul < ARRAYITEMCOUNT(ausVolumeIDs);
                 ul++)
                WinEnableControl(hwndDlg, ausVolumeIDs[ul], fVolumeChecked);

            WinEnableControl(hwndDlg,
                             DID_OK,
                             (    (fVolumeChecked)
                               || (fEnable)
                             ));
        }
        break;
#endif

        default:
            mrc = WinDefDlgProc(hwndDlg, msg, mp1, mp2);
    }

    return (mrc);
}

/*
 *@@ ConfirmCreateVolumeFromFreespace:
 *
 *      pData->pPartitionSource _must_ contain the
 *      current partition on which to work.
 *
 *      Returns TRUE if the partition was created and
 *      the given LVMDATA must be refreshed.
 *
 *      Dialog design:
 *
 *      1)  Select partition and volume type:
 *
 *          --  Create a volume with a primary partition. That is
 *              bootable by definition.
 *
 *          --  Create a volume with a logical drive partition. In
 *              that case, the volume can be LVM or compatibility.
 *
 *      2)  Select partition size and whether to allocate the
 *          partition from the start or the end of the free space.
 *
 *      3)  Enter the partition and volume names. For simplicity,
 *          we'll set the same for the two.
 *
 *      4)  Select the drive letter or none.
 *
 *@@added V0.9.12 (2001-05-10) [umoeller]
 */

VOID ConfirmCreateVolumeFromFreespace(PLVMDATA pData)
{
    CHAR    szPartitionAndVolumeName[100];

    if (    // we must have a partition to start with
            (pData->pPartitionSource)
            // the partition must be freespace
         && (pData->pPartitionSource->Partition_Type == 0)
       )
    {
        HWND hwndDlg;

        #define CX_EXPL         250

        static CONTROLDEF
            TypeGroup = CONTROLDEF_GROUP(
                            "1) Select partition and volume type",
                            ID_CREATE_TYPEGROUP,
                            -100,
                            SZL_AUTOSIZE),
            Spacing = CONTROLDEF_TEXT(
                            " ",
                            10,
                            10,
                            SZL_AUTOSIZE),
            PrimaryRadio = CONTROLDEF_FIRST_AUTORADIO(
                            "Bootable primary partition",
                            ID_CREATE_PRIMARY_RADIO,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
            PrimaryExpl = CONTROLDEF_TEXT_WORDBREAK(
                            "Some operating systems other than OS/2 may have to be installed on a primary partition. "
                            "Note that there can be no more than four primary partitions on a disk.",
                            -1,
                            CX_EXPL),
            LogicalBootableRadio = CONTROLDEF_NEXT_AUTORADIO(
                            "Bootable volume on logical drive",
                            ID_CREATE_LOGICALBOOTABLE_RADIO,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
            LogicalBootableExpl = CONTROLDEF_TEXT_WORDBREAK(
                            "This type must be used for OS/2 boot volumes.",
                            -1,
                            CX_EXPL),
            LogicalCompatRadio = CONTROLDEF_NEXT_AUTORADIO(
                            "Non-bootable compatibility volume on logical drive",
                            ID_CREATE_COMPAT_RADIO,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
            LogicalCompatExpl = CONTROLDEF_TEXT_WORDBREAK(
                            "Use this type if you do not boot from this volume, but want the volume to "
                            "be accessible from other operating systems. Note that you cannot use JFS "
                            "on a compatibility volume.",
                            -1,
                            CX_EXPL),
            LogicalLVMRadio = CONTROLDEF_NEXT_AUTORADIO(
                            "Non-bootable LVM volume on logical drive",
                            ID_CREATE_LVM_RADIO,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
            LogicalLVMExpl = CONTROLDEF_TEXT_WORDBREAK(
                            "Use this type if you want to use JFS on this volume. Note that other operating "
                            "systems will not be able to see this volume.",
                            -1,
                            CX_EXPL),
            SizeTxt = CONTROLDEF_TEXT(
                            "2) Set the partition size:",
                            ID_CREATE_SIZETXT,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE);
        SLDCDATA SizeSliderCData =
             {
                 sizeof(SLDCDATA),
                 100,       // scale 1 increments
                 0,         // scale 1 spacing
                 1,         // scale 2 increments
                 0          // scale 2 spacing
             };

        static CONTROLDEF
            SizeSlider = CONTROLDEF_SLIDER(
                            ID_CREATE_SIZESLIDER,
                            100,
                            20,
                            &SizeSliderCData),
            SizeEF = CONTROLDEF_ENTRYFIELD(
                            NULL,       // set explicitly
                            ID_CREATE_SIZEENTRYFIELD,
                            40,
                            SZL_AUTOSIZE),
            SizeMB = CONTROLDEF_TEXT(
                            "MB",
                            ID_CREATE_SIZEMB,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
            NameTxt = CONTROLDEF_TEXT(
                            "3) Enter the volume name:",
                            ID_CREATE_NAMETXT,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
            NameEF = CONTROLDEF_ENTRYFIELD(
                            NULL,
                            ID_CREATE_NAMEEF,
                            SZL_REMAINDER,
                            SZL_AUTOSIZE),
            DriveLetterTxt = CONTROLDEF_TEXT(
                            "4) Select a drive letter for this volume:",
                            ID_CREATE_LETTERTXT,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
            DriveLetterDrop = CONTROLDEF_DROPDOWNLIST(
                            ID_CREATE_LETTERDROP,
                            30,
                            100);
        static DLGHITEM dlgCreateVolumeFromFreespace[] =
            {
                START_TABLE,
                    START_ROW(0),
                        START_TABLE_EXT(TABLE_ALIGN_COLUMNS | TABLE_INHERIT_SIZE),
                            // 1) partition and volume type
                            START_ROW(ROW_VALIGN_CENTER),
                                START_GROUP_TABLE_EXT(&TypeGroup, TABLE_INHERIT_SIZE),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&PrimaryRadio),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&Spacing),
                                        CONTROL_DEF(&PrimaryExpl),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&LogicalBootableRadio),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&Spacing),
                                        CONTROL_DEF(&LogicalBootableExpl),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&LogicalCompatRadio),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&Spacing),
                                        CONTROL_DEF(&LogicalCompatExpl),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&LogicalLVMRadio),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&Spacing),
                                        CONTROL_DEF(&LogicalLVMExpl),
                                END_TABLE,
                            START_ROW(0),
                                START_TABLE_EXT(TABLE_ALIGN_COLUMNS | TABLE_INHERIT_SIZE),
                                    // 2) partition size
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&SizeTxt),
                                        CONTROL_DEF(&SizeSlider),
                                        CONTROL_DEF(&SizeEF),
                                        CONTROL_DEF(&SizeMB),
                                    // 3) partition and volume names
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&NameTxt),
                                        CONTROL_DEF(&NameEF),
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&DriveLetterTxt),
                                        CONTROL_DEF(&DriveLetterDrop),
                                END_TABLE,
                            START_ROW(0),
                                START_TABLE,
                                    START_ROW(ROW_VALIGN_CENTER),
                                        CONTROL_DEF(&G_OKButton),
                                        CONTROL_DEF(&G_CancelButton),
                                END_TABLE,
                        END_TABLE,
                END_TABLE
            };

        strcpy(szPartitionAndVolumeName,
               pData->pPartitionSource->Partition_Name);
                        // this is preset by LVM

        if (!dlghCreateDlg(&hwndDlg,
                           pData->hwndClient,
                           FCF_FIXED_DLG,
                           fnwpConfirmCreateDlg,
                           "Create partition/volume",
                           dlgCreateVolumeFromFreespace,
                           ARRAYITEMCOUNT(dlgCreateVolumeFromFreespace),
                           pData,
                           G_pcszDefaultFont))
        {
            CARDINAL32 Error;
            ULONG ulOptions = Get_Valid_Options(pData->pPartitionSource->Partition_Handle,
                                                &Error);
            ULONG idSelect = ID_CREATE_COMPAT_RADIO;

            // enable the items that are available
            if (!(ulOptions & CREATE_PRIMARY_PARTITION))
                WinEnableControl(hwndDlg, ID_CREATE_PRIMARY_RADIO, FALSE);
            // enable the items that are available
            if (!(ulOptions & CREATE_LOGICAL_DRIVE))
            {
                WinEnableControl(hwndDlg, ID_CREATE_LOGICALBOOTABLE_RADIO, FALSE);
                WinEnableControl(hwndDlg, ID_CREATE_LVM_RADIO, FALSE);
                WinEnableControl(hwndDlg, ID_CREATE_COMPAT_RADIO, FALSE);
            }

            WinCheckButton(hwndDlg, idSelect, TRUE);
            WinSetFocus(HWND_DESKTOP, WinWindowFromID(hwndDlg, idSelect));

            HWND hwndSlider = WinWindowFromID(hwndDlg, ID_CREATE_SIZESLIDER);
            winhSetSliderTicks(hwndSlider,
                               MPFROM2SHORT(4, 5), 3,
                               MPFROM2SHORT(9, 10), 6);

            winhSetSliderArmPosition(hwndSlider, SMA_INCREMENTVALUE, 99);       // 100%

            // WinPostMsg(hwndDlg, XM_ENABLEITEMS, 0, 0);

            winhCenterWindow(hwndDlg);
            WinProcessDlg(hwndDlg);

            WinDestroyWindow(hwndDlg);
        }
    }
}

/*
 *@@ CreateVolumeString:
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

PCSZ CreateVolumeString(PSZ pszBuf,
                        PLVMDATA pData)
{
    if (pData->pPartitionSource->Volume_Handle)
    {
        // the volume can be hidden and have no drive letter
        if (pData->pPartitionSource->Volume_Drive_Letter)
            sprintf(pszBuf,
                    "the volume %c: (\"%s\"), which is "
                    "assigned to partition %d (\"%s\"), ",
                    pData->pPartitionSource->Volume_Drive_Letter,
                    pData->pPartitionSource->Volume_Name,
                    lstIndexFromItem(&pData->llPartitions,
                                     pData->pPartitionSource),
                    pData->pPartitionSource->Partition_Name);
        else
            sprintf(pszBuf,
                    "the hidden volume \"%s\", which is "
                    "assigned to partition %d (\"%s\"), ",
                    pData->pPartitionSource->Volume_Name,
                    lstIndexFromItem(&pData->llPartitions,
                                     pData->pPartitionSource),
                    pData->pPartitionSource->Partition_Name);
    }
    else
        // no volume:
        sprintf(pszBuf,
                "the partition \"%s\" (which is "
                "currently not assigned to any volume)",
                pData->pPartitionSource->Volume_Name,
                lstIndexFromItem(&pData->llPartitions,
                                 pData->pPartitionSource),
                pData->pPartitionSource->Partition_Name);

    return pszBuf;
}

/*
 *@@ ConfirmDelete:
 *      displays a confirmation box and deletes
 *      the specified partition/volume.
 *
 *      pData->pPartitionSource _must_ contain the
 *      current partition on which to work.
 *
 *      If the current selection is a volume,
 *      this deletes the entire volume, including
 *      all partitions that belong to it
 *      (LVM Delete_Volume call).
 *
 *      If the current selection is a partition
 *      that is not part of a volume, this deletes
 *      the partition only.
 *
 *@@added V0.9.12 (2001-05-10) [umoeller]
 */

VOID ConfirmDelete(PLVMDATA pData)
{
    XSTRING str;
    xstrInit(&str, 0);

    if (pData->pPartitionSource)
    {
        if (pData->pPartitionSource->Volume_Handle)
        {
            // the volume can be hidden and have no drive letter
            if (pData->pPartitionSource->Volume_Drive_Letter)
                xstrPrintf(&str,
                        "You have selected to delete the volume %c: (\"%s\"), which is "
                        "assigned to partition %d (\"%s\")."
                        "\n\nDeleting the volume will completely erase all the data that is "
                        "presently stored on it, including the partition that it was assigned to. "
                        "This cannot be undone once xlvm commits your changes to disk on exit. "
                        "\n\nAre you sure you want to do this?",
                        pData->pPartitionSource->Volume_Drive_Letter,
                        pData->pPartitionSource->Volume_Name,
                        lstIndexFromItem(&pData->llPartitions,
                                         pData->pPartitionSource),
                        pData->pPartitionSource->Partition_Name);
            else
                xstrPrintf(&str,
                        "You have selected to delete the hidden volume \"%s\", which is "
                        "assigned to partition %d (\"%s\"). "
                        "\n\nEven though the data on this volume is currently not visible to OS/2, "
                        "deleting the volume will completely erase all the data that is "
                        "presently stored on it, including the partition that it was assigned to. "
                        "This cannot be undone once xlvm commits your changes to disk on exit. "
                        "\n\nAre you sure you want to do this?",
                        pData->pPartitionSource->Volume_Name,
                        lstIndexFromItem(&pData->llPartitions,
                                         pData->pPartitionSource),
                        pData->pPartitionSource->Partition_Name);
        }
        else
            // no volume:
            xstrPrintf(&str,
                    "You have selected to delete the partition \"%s\", which is "
                    "currently not assigned to any volume. "
                    "\n\nEven though the data on this partition is currently not visible to OS/2, "
                    "deleting the partition will completely erase all the data that is "
                    "presently stored on it. "
                    "This cannot be undone once xlvm commits your changes to disk on exit. "
                    "\n\nAre you sure you want to do this?",
                    pData->pPartitionSource->Volume_Name,
                    lstIndexFromItem(&pData->llPartitions,
                                     pData->pPartitionSource),
                    pData->pPartitionSource->Partition_Name);

        if (MBID_YES == MessageBox(pData->hwndClient,
                                   &str,
                                   MB_YESNO | MB_DEFBUTTON2))
        {
            CARDINAL32 Error = 0;

            // we can't delete partitions if they're part of a volume,
            // so check volume first
            if (pData->pPartitionSource->Volume_Handle)
            {
                Delete_Volume(pData->pPartitionSource->Volume_Handle,
                              &Error);
                if (Error)
                    LVMError(pData->hwndClient, Error, "Delete_Volume");
            }
            else
            {
                // no volume, but partition:
                Delete_Partition(pData->pPartitionSource->Partition_Handle,
                                 &Error);

                if (Error)
                    LVMError(pData->hwndClient, Error, "Delete_Partition");
            }

            if (Changes_Pending())
                pData->fDirty = TRUE;

            // refresh display
            WinPostMsg(pData->hwndClient, XM_INSERTDATA, 0, 0);
        }
    }

    xstrClear(&str);
}

/*
 *@@ ConfirmHideVolume:
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

VOID ConfirmHideVolume(PLVMDATA pData)
{
    CARDINAL32 Error = 0;
    XSTRING str;
    xstrInit(&str, 0);
    if (pData->pPartitionSource)
    {
        xstrPrintf(&str,
                "This will make volume %c: (\"%s\") inaccessible to OS/2. "
                "Even though no data will be deleted, "
                "the drive letter that was assigned will no longer exist, and "
                "all files on that drive can no longer be seen from OS/2. "
                "\n\nAre you sure you want to do this?",
                pData->pPartitionSource->Volume_Drive_Letter,
                pData->pPartitionSource->Volume_Name);
        if (MBID_YES == MessageBox(pData->hwndClient,
                                   &str,
                                   MB_YESNO | MB_DEFBUTTON2))
        {
            Hide_Volume(pData->pPartitionSource->Volume_Handle,
                        &Error);
            if (Error)
                LVMError(pData->hwndClient,
                         Error,
                         "Hide_Volume");
            else
                // if (Changes_Pending())       this is not reported!!!
                    pData->fDirty = TRUE;

            // refresh display
            WinPostMsg(pData->hwndClient, XM_INSERTDATA, 0, 0);
        }
    }

    xstrClear(&str);
}

/*
 *@@ ConfirmAddToBootMgr:
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

VOID ConfirmAddToBootMgr(PLVMDATA pData)
{
    CARDINAL32 Error = 0;
    XSTRING str;
    CHAR sz2[1000];
    xstrInit(&str, 0);

    if (pData->pPartitionSource)
    {
        xstrPrintf(&str,
                "This will add %s to the boot manager menu. "
                "\n\nAre you sure you want to do this?",
                CreateVolumeString(sz2, pData));
        if (MBID_YES == MessageBox(pData->hwndClient,
                                   &str,
                                   MB_YESNO | MB_DEFBUTTON2))
        {
            Add_To_Boot_Manager(// pData->pPartitionSource->Partition_Handle,
                                pData->pPartitionSource->Volume_Handle,
                                &Error);
            if (Error)
                LVMError(pData->hwndClient,
                         Error,
                         "Add_To_Boot_Manager");

            if (Changes_Pending())
                pData->fDirty = TRUE;
        }
    }

    xstrClear(&str);
}

/*
 *@@ ConfirmRemoveFromBootMgr:
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

VOID ConfirmRemoveFromBootMgr(PLVMDATA pData)
{
    CARDINAL32 Error = 0;
    XSTRING str;
    CHAR sz2[1000];
    xstrInit(&str, 0);

    if (pData->pPartitionSource)
    {
        xstrPrintf(&str,
                "This will remove %s from the boot manager menu. "
                "\n\nAre you sure you want to do this?",
                CreateVolumeString(sz2, pData));
        if (MBID_YES == MessageBox(WinQueryWindow(pData->hwndClient, QW_OWNER),
                                   &str,
                                   MB_YESNO | MB_DEFBUTTON2))
        {
            Remove_From_Boot_Manager(// pData->pPartitionSource->Partition_Handle,
                                     pData->pPartitionSource->Volume_Handle,
                                     &Error);
            if (Error)
                LVMError(pData->hwndClient,
                         Error,
                         "Remove_From_Boot_Manager");

            if (Changes_Pending())
                pData->fDirty = TRUE;
        }
    }

    xstrClear(&str);
}

/*
 *@@ ShowProductInfo:
 *
 *@@added V1.0.0 (2002-08-12) [umoeller]
 */

VOID ShowProductInfo(HWND hwndClient)
{
    #define PI_WIDTH 100

    static const CONTROLDEF
        PIIcon = CONTROLDEF_ICON(
                        G_hptrDrive,
                        1),
        PINull = CONTROLDEF_TEXT("", 999, 1, 1),
        PITopText = CONTROLDEF_TEXT_WORDBREAK(
                        "xlvm " XFOLDER_VERSION "\n"
                        "Built " __DATE__,
                        2,
                        PI_WIDTH),
        PISep = CONTROLDEF_SEPARATORLINE(
                        3,
                        PI_WIDTH,
                        4),
        PIBottomText = CONTROLDEF_TEXT_WORDBREAK(
                        "(C) 2001-2003 Ulrich M”ller",
                        5,
                        PI_WIDTH);

    static const DLGHITEM aDlg[] =
        {
            START_TABLE_ALIGN,
                START_ROW(ROW_VALIGN_CENTER),
                    START_TABLE,
                        START_ROW(0),
                            CONTROL_DEF(&PIIcon),
                    END_TABLE,
                    START_TABLE,
                        START_ROW(0),
                            CONTROL_DEF(&PITopText),
                        START_ROW(0),
                            CONTROL_DEF(&PISep),
                        START_ROW(0),
                            CONTROL_DEF(&PIBottomText),
                    END_TABLE,
                START_ROW(ROW_VALIGN_CENTER),
                    CONTROL_DEF(&PINull),
                    CONTROL_DEF(&G_OKButton),
            END_TABLE,
        };

    HWND hwndDlg;
    if (!dlghCreateDlg(&hwndDlg,
                       hwndClient,
                       FCF_FIXED_DLG,
                       fnwpConfirmCreateDlg,
                       "xlvm Product Info",
                       aDlg,
                       ARRAYITEMCOUNT(aDlg),
                       NULL,
                       G_pcszDefaultFont))
    {
        winhCenterWindow(hwndDlg);
        WinProcessDlg(hwndDlg);
        WinDestroyWindow(hwndDlg);
    }
}

/*
 *@@ ClientCommand:
 *      command dispatcher for WM_COMMAND in fnwpClient.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

VOID ClientCommand(HWND hwndClient,
                   MPARAM mp1)
{
    PLVMDATA pData = (PLVMDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

    SHORT sCommand =  SHORT1FROMMP(mp1);
    switch (sCommand)
    {
        // "file" menu
        case IDMI_EXIT:
            WinPostMsg(WinQueryWindow(hwndClient, QW_OWNER),
                       WM_SYSCOMMAND,
                       (MPARAM)SC_CLOSE,
                       0);
        break;

        // "view" menu
        case IDMI_VIEW_BARCHART:
        case IDMI_VIEW_PIECHART:
        {
            G_ulViewStyle = sCommand;

            PLISTNODE pNode;
            for (pNode = lstQueryFirstNode(&pData->llDriveData);
                 pNode;
                 pNode = pNode->pNext)
            {
                PLVMDRIVEDATA pDriveData = (PLVMDRIVEDATA)pNode->pItemData;
                if (pDriveData->hwndChart)
                {
                    SetChartStyle(pDriveData->hwndChart);
                }
            }

            RefreshViewMenu(WinQueryWindow(hwndClient, QW_PARENT));
        }
        break;

        case IDMI_VIEW_REFRESH:
            WinPostMsg(hwndClient,
                       XM_INSERTDATA,
                       0,
                       0);
        break;

        // whitespace context menu
        case IDMI_INSTALLBMGR:
            NotImplemented(hwndClient);
        break;

        case IDMI_REMOVEBMGR:
            NotImplemented(hwndClient);
        break;

        case IDMI_SETMGRVALUES:
            NotImplemented(hwndClient);
        break;

        /*
         * IDMI_FREESPACE_CREATEVOLUME:
         *      "create volume" item from freespace
         *      context menu.
         */

        case IDMI_FREESPACE_CREATEVOLUME:
            ConfirmCreateVolumeFromFreespace(pData);
        break;

        case IDMI_PARTITION_CREATEVOLUME:
            NotImplemented(hwndClient);
        break;

        /*
         * IDMI_DELETE:
         *      can come from
         *
         *      --  partition (no volume assigned)
         *
         *      --  volume
         */

        case IDMI_DELETE:
            ConfirmDelete(pData);
        break;

        case IDMI_RENAME:
            NotImplemented(hwndClient);
        break;

        // volume context menu
        case IDMI_EXPANDVOLUME:
            NotImplemented(hwndClient);
        break;

        case IDMI_HIDEVOLUME:
            ConfirmHideVolume(pData);
        break;

        case IDMI_CHANGELETTER:
            NotImplemented(hwndClient);
        break;

        case IDMI_RENAMEPARTITION_VOLUME:
            NotImplemented(hwndClient);
        break;

        case IDMI_SETSTARTABLE:
            NotImplemented(hwndClient);
        break;

        case IDMI_ADDTOBMGR:
            ConfirmAddToBootMgr(pData);
        break;

        case IDMI_REMOVEFROMBMGR:
            ConfirmRemoveFromBootMgr(pData);
        break;

        case IDMI_HELP_PRODINFO:
            ShowProductInfo(hwndClient);
        break;
    }
}

/* ******************************************************************
 *
 *   Client window proc
 *
 ********************************************************************/

/*
 *@@ GetPartitionColor:
 *      returns a color according to the partition type.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

LONG GetPartitionColor(ULONG ulType,
                       Partition_Information_Record *pBarItem, // in: partition info
                       PSZ pszType)       // out: if != NULL, description
{
    switch (ulType)
    {
        case 0:     // free
            if (pszType)
                strcpy(pszType, "Free space");
            return RGBCOL_DARKGRAY;

        case 0x14:
        case 0x06:
        case 0x01:
        case 0x04:
        case 0x84:  // FAT
            if (pszType)
                strcpy(pszType, "FAT16");
            return RGBCOL_DARKGREEN;

        case 0x07: // compatibility volume
        case 0x35:  // LVM volume
            if (pszType)
            {
                if (    (pBarItem)
                     && (pBarItem->Volume_Drive_Letter)
                     && (!doshQueryDiskFSType(toupper(pBarItem->Volume_Drive_Letter) - 'A' + 1,
                                              pszType,
                                              20))
                   )
                    ;
                else
                    strcpy(pszType, "unknown");
            }

            if (ulType == 0x07)
                return RGBCOL_DARKBLUE;

            return RGBCOL_BLUE;

        case 0x0b:
        case 0x0c:  // FAT32
            if (pszType)
                strcpy(pszType, "FAT32");
            return RGBCOL_DARKYELLOW;

        case 0x82:
            if (pszType)
                strcpy(pszType, "Linux swap");
            return RGBCOL_DARKRED;

        case 0x83:  // linux
            if (pszType)
                strcpy(pszType, "Linux ext2fs");
            return RGBCOL_DARKRED;

        case 0x0a:  // boot manager
            if (pszType)
                strcpy(pszType, "IBM boot manager");
            return RGBCOL_DARKCYAN;

    }

    // unknown:
    if (pszType)
        strcpy(pszType, "unknown");

    return RGBCOL_DARKGRAY;
}

/*
 *@@ SetControlsData:
 *      creates and sets up the chart controls in the client.
 *
 *      The chart controls maintain full copies of the display
 *      data, so there's nothing to free here afterwards.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

VOID SetControlsData(HWND hwndClient)
{
    PLVMDATA pData = (PLVMDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

    // count drives; we need to divide the remaining space
    ULONG   cDrives = lstCountItems(&pData->llDriveData);
    ULONG   ulCurrentID = 1000;

    if (cDrives)        // avoid division by zero
    {
        // drives loop
        PLISTNODE pNode = lstQueryFirstNode(&pData->llDriveData);
        while (pNode)
        {
            PLVMDRIVEDATA pDriveData = (PLVMDRIVEDATA)pNode->pItemData;

            ULONG ulDriveNo = pDriveData->pControlRecord->Drive_Number;

            // create a chart control for this
            if (!pDriveData->hwndChart)
            {
                LONG lColor = RGBCOL_WHITE;
                pDriveData->hwndChart = winhCreateControl(hwndClient,
                                                          hwndClient,
                                                          WC_STATIC,
                                                          "chart",
                                                          SS_TEXT | DT_LEFT | DT_VCENTER
                                                            | WS_VISIBLE,
                                                          // ID:
                                                          ulCurrentID++);
                WinSetPresParam(pDriveData->hwndChart,
                                PP_FOREGROUNDCOLOR,
                                sizeof(lColor),
                                &lColor);
                lColor = WinQuerySysColor(HWND_DESKTOP,
                                          SYSCLR_APPWORKSPACE,
                                          0);
                WinSetPresParam(pDriveData->hwndDescription,
                                PP_BACKGROUNDCOLOR,
                                sizeof(lColor),
                                &lColor);
                ctlChartFromStatic(pDriveData->hwndChart);

                SetChartStyle(pDriveData->hwndChart);
            }

            // and a description static
            if (!pDriveData->hwndDescription)
            {
                pDriveData->hwndDescription = winhCreateControl(hwndClient,
                                                                hwndClient,
                                                                WC_STATIC,
                                                                "descr",
                                                                SS_TEXT | DT_LEFT | DT_VCENTER
                                                                    | WS_VISIBLE,
                                                                0);
                LONG lColor = WinQuerySysColor(HWND_DESKTOP,
                                               SYSCLR_APPWORKSPACE,
                                               0);
                WinSetPresParam(pDriveData->hwndDescription,
                                PP_BACKGROUNDCOLOR,
                                sizeof(lColor),
                                &lColor);
            }

            // set drive description text
            ULONG ulMB =    MB_FROM_SECTORS(pDriveData->pControlRecord->Drive_Size);
            CHAR sz[1000], sz2[50], sz3[50];
            LONG lSectorsOnDrive = pDriveData->pControlRecord->Drive_Size;

            sprintf(sz,
                    "Drive %d: \"%s\", %s MBytes, %s sectors (CHS: %d/%d/%d)",
                    ulDriveNo,
                    pDriveData->InfoRecord.Drive_Name,
                    nlsThousandsULong(sz2,
                                      ulMB,
                                      G_CountrySettings.cs.cThousands),
                    nlsThousandsULong(sz3,
                                      lSectorsOnDrive,
                                      G_CountrySettings.cs.cThousands),
                    pDriveData->pControlRecord->Cylinder_Count,
                    pDriveData->pControlRecord->Heads_Per_Cylinder,
                    pDriveData->pControlRecord->Sectors_Per_Track);
            WinSetWindowText(pDriveData->hwndDescription, sz);

            // set chart data
            ULONG cPartitionsThis = pDriveData->PartitionsArray.Count;
            CHARTDATA       cd;
            ZERO(&cd);
            cd.usStartAngle = 90 + 45/2;
            cd.usSweepAngle = 360 -  45;
            cd.cValues = cPartitionsThis; // array count
            cd.padValues = (double*)malloc(sizeof(double) * cPartitionsThis);
            cd.palColors = (long*)malloc(sizeof(long) * cPartitionsThis);
            cd.papszDescriptions = (PSZ*)malloc(sizeof(PSZ) * cPartitionsThis);

            LONG lMinSize = (lSectorsOnDrive / 1000) * 2 / 100;

            ULONG ul;
            for (ul = 0;
                 ul < cPartitionsThis;
                 ul++)
            {
                Partition_Information_Record *pBarItem
                            = &pDriveData->PartitionsArray.Partition_Array[ul];

                LONG lSizeThis = pBarItem->True_Partition_Size / 1000;
                // if this is a small partition (less than 2% of the disk
                // size), enlarge it to 2% of the disk size
                if (lSizeThis < lMinSize)
                    lSizeThis = lMinSize;

                cd.padValues[ul] = (double)lSizeThis;

                // set color
                ULONG ulType = 0;
                if (pBarItem->Partition_Type != 0)
                    ulType = pBarItem->OS_Flag;
                cd.palColors[ul] = GetPartitionColor(ulType, NULL, NULL);

                // set text
                sprintf(sz,
                        "%u",
                        lstIndexFromItem(&pData->llPartitions, pBarItem));
                if (pBarItem->Volume_Drive_Letter)
                    sprintf(sz + strlen(sz),
                            "\n%c:",
                            pBarItem->Volume_Drive_Letter);

                cd.papszDescriptions[ul] = strdup(sz);
            }

            // cd.papszDescriptions = &apszDescriptions[0];
            WinSendMsg(pDriveData->hwndChart, CHTM_SETCHARTDATA, &cd, NULL);

            free(cd.padValues);
            free(cd.palColors);
            for (ul = 0;
                 ul < cPartitionsThis;
                 ul++)
                free(cd.papszDescriptions[ul]);
            free(cd.papszDescriptions);

            // next drive
            pNode = pNode->pNext;
        }
    }
}

/*
 *@@ PositionChartsInClient:
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

VOID PositionChartsInClient(HWND hwndClient)
{
    PLVMDATA pData = (PLVMDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

    RECTL rclClient;

    WinQueryWindowRect(hwndClient,
                       &rclClient); // exclusive

    #define OUTER_SPACING 20
    #define INNER_SPACING 10
    #define DESCR_HEIGHT  20

    // reduce the rect by 20
    gpihInflateRect(&rclClient, -OUTER_SPACING);

    // count drives; we need to divide the remaining space
    ULONG cDrives = lstCountItems(&pData->llDriveData);

    if (cDrives)        // avoid division by zero
    {
        LONG cyPaint = rclClient.yTop - rclClient.yBottom;
        // leave another 20 pixels space between the drives
        cyPaint -= (INNER_SPACING * (cDrives - 1));
        LONG cyPerDrive = cyPaint / cDrives;

        // start with drive 1 on top
        LONG yTop = rclClient.yTop;

        // drives loop
        PLISTNODE pNode = lstQueryFirstNode(&pData->llDriveData);
        while (pNode)
        {
            PLVMDRIVEDATA pDriveData = (PLVMDRIVEDATA)pNode->pItemData;

            WinSetWindowPos(pDriveData->hwndChart,
                            0,
                            rclClient.xLeft,
                            yTop - cyPerDrive + INNER_SPACING + DESCR_HEIGHT,
                            rclClient.xRight - rclClient.xLeft,
                            cyPerDrive - INNER_SPACING - DESCR_HEIGHT,
                            SWP_MOVE | SWP_SIZE);

            WinSetWindowPos(pDriveData->hwndDescription,
                            0,
                            rclClient.xLeft,
                            yTop - cyPerDrive,
                            rclClient.xRight - rclClient.xLeft,
                            DESCR_HEIGHT,
                            SWP_MOVE | SWP_SIZE);

            // next drive
            pNode = pNode->pNext;
            // go down for next drive
            yTop -= cyPerDrive;
            yTop -= INNER_SPACING;
        }
    }
}

/*
 *@@ GetVolumeDescription:
 *      describes the given volume.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

VOID GetVolumeDescription(PLVMDATA pData,
                          PSZ psz,      // out: description string
                          Partition_Information_Record *pBarItem) // in: partition info
{
    *psz = '\0';

    psz += sprintf(psz,
                   "Partition %u",
                   lstIndexFromItem(&pData->llPartitions, pBarItem));

    if (pBarItem->Partition_Type == 0)
        psz += sprintf(psz, ": free space");
    else
    {
        if (pBarItem->Volume_Drive_Letter)
            psz += sprintf(psz, ": drive %c:", pBarItem->Volume_Drive_Letter);
        else
            psz += sprintf(psz, ": hidden");

        if (strlen(pBarItem->Volume_Name))
            psz += sprintf(psz,
                           " (Volume name \"%s\") ",
                           pBarItem->Volume_Name);

        if (pBarItem->Primary_Partition)
            psz += sprintf(psz, ", primary partition");

        CHAR szDesc[100];
        GetPartitionColor(pBarItem->OS_Flag, pBarItem, szDesc);

        if (pBarItem->OS_Flag)
            psz += sprintf(psz,
                           ", type 0x%lX (%s)",
                           pBarItem->OS_Flag,
                           szDesc);
    }

    CHAR sz2[200], sz3[200];

    psz += sprintf(psz,
                   ", %s MB (%s sectors)",
                   nlsThousandsULong(sz2,
                                     MB_FROM_SECTORS(pBarItem->True_Partition_Size),
                                     G_CountrySettings.cs.cThousands),
                   nlsThousandsULong(sz3,
                                     pBarItem->True_Partition_Size,
                                     G_CountrySettings.cs.cThousands));
}

/*
 *@@ ShowContextMenu:
 *      builds and shows a context menu for the
 *      selected partition.
 *
 *@@added V0.9.12 (2001-05-03) [umoeller]
 */

VOID ShowContextMenu(PLVMDATA pData,
                     PEMPHASISNOTIFY pen,
                     Partition_Information_Record *pBarItem)
{
    HWND hwndMenu = 0;

    if (!pBarItem)
        hwndMenu = G_hwndWhitespaceMenu;
    else
        if (pBarItem->Partition_Type == 0)
        {
            hwndMenu = G_hwndFreeSpaceMenu;

            // now enable/disable the items that don't apply
            CARDINAL32 Error = 0;
            ULONG ulOptions = Get_Valid_Options(pBarItem->Partition_Handle,
                                                &Error);

            WinEnableMenuItem(hwndMenu,
                              IDMI_PARTITION_CREATEVOLUME,
                              (    ulOptions
                                   & (CREATE_PRIMARY_PARTITION | CREATE_LOGICAL_DRIVE))
                              != 0);
            WinEnableMenuItem(hwndMenu,
                              IDMI_FREESPACE_CREATEVOLUME,
                              (    ulOptions
                                   & (CREATE_PRIMARY_PARTITION | CREATE_LOGICAL_DRIVE))
                              != 0);
        }
        else
        {
            if (pBarItem->Volume_Handle)
            {
                // this partition is part of a volume:
                hwndMenu = G_hwndVolumeMenu;

                // now enable/disable the items that don't apply
                CARDINAL32 Error = 0;
                ULONG ulOptions = Get_Valid_Options(pBarItem->Volume_Handle,
                                                          &Error);

                WinEnableMenuItem(hwndMenu,
                                  IDMI_EXPANDVOLUME,
                                  (ulOptions & EXPAND_VOLUME) != 0);
                WinEnableMenuItem(hwndMenu,
                                  IDMI_DELETE,
                                  (ulOptions & DELETE_VOLUME) != 0);
                WinEnableMenuItem(hwndMenu,
                                  IDMI_HIDEVOLUME,
                                  (ulOptions & HIDE_VOLUME) != 0);
                WinEnableMenuItem(hwndMenu,
                                  IDMI_CHANGELETTER,
                                  (ulOptions & ASSIGN_DRIVE_LETTER) != 0);

                // rename partition/volume is always enabled

                WinEnableMenuItem(hwndMenu,
                                  IDMI_SETSTARTABLE,
                                  (ulOptions & SET_STARTABLE) != 0);
                WinEnableMenuItem(hwndMenu,
                                  IDMI_ADDTOBMGR,
                                  (ulOptions & ADD_TO_BOOT_MANAGER_MENU) != 0);
                WinEnableMenuItem(hwndMenu,
                                  IDMI_REMOVEFROMBMGR,
                                  (ulOptions & REMOVE_FROM_BOOT_MANAGER_MENU) != 0);
            }
            else
                // partition not currently assigned to a volume:
                hwndMenu = G_hwndPartitionMenu;
        }

    if (hwndMenu)
    {
        pData->pPartitionSource = pBarItem;
        G_hwndMenuShowing = hwndMenu;       // to remove source emphasis later

        POINTL ptl = {pen->ptl.x, pen->ptl.y};
        // map to desktop
        WinMapWindowPoints(pen->hwndSource,     // from chart ctl
                           HWND_DESKTOP,
                           &ptl,
                           1);

        // add source emphasis
        WinSendMsg(pen->hwndSource,
                   CHTM_SETEMPHASIS,
                   (MPARAM)1,           // source emphasis
                   (MPARAM)pen->lIndex);

        // store emphasis data
        memcpy(&G_CurrentEmphasis, pen, sizeof(EMPHASISNOTIFY));

        WinPopupMenu(HWND_DESKTOP,
                     pData->hwndClient,
                     hwndMenu,
                     ptl.x,
                     ptl.y,
                     0,
                     PU_HCONSTRAIN | PU_VCONSTRAIN | PU_MOUSEBUTTON1
                        | PU_MOUSEBUTTON2 | PU_KEYBOARD);
    }
}

/*
 *@@ ConfirmCommit:
 *
 *@@added V0.9.18 (2002-03-23) [umoeller]
 */

BOOL ConfirmCommit(HWND hwndClient)
{
    BOOL fClose = FALSE;

    XSTRING str;
    xstrInitCopy(&str,
                 "Changes have been made to the disk setup. "
                 "At this point, none of the changes have been written to the disk(s) yet. "
                 "Writing the changes will make them permanent. "
                 "\n\nShould all pending changes be written to disk now?",
                 0);

    ULONG ulrc = MessageBox(hwndClient,
                            &str,
                            MB_YESNOCANCEL | MB_DEFBUTTON2);
    switch (ulrc)
    {
        case MBID_YES:
        {
            // commit:
            CARDINAL32 Error;
            if (Commit_Changes(&Error))
                fClose = TRUE;
            else
                LVMError(hwndClient,
                         Error,
                         "Commit_Changes");
        }
        break;

        case MBID_NO:
            fClose = TRUE;
        break;
    }

    xstrClear(&str);

    return fClose;
}

/*
 *@@ fnwpClient:
 *      client window proc.
 */

MRESULT EXPENTRY fnwpClient(HWND hwndClient, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CREATE:
        {
            PLVMDATA pData = NEW(LVMDATA);
            ZERO(pData);

            lstInit(&pData->llDriveData, FALSE);
            lstInit(&pData->llPartitions, FALSE);

            pData->hwndClient = hwndClient;

            WinSetWindowPtr(hwndClient, QWL_USER, pData);

            WinPostMsg(hwndClient,
                       XM_INSERTDATA,
                       0,
                       0);
        }
        break;

        /*
         * XM_INSERTDATA:
         *      posted on startup to load the LVM data.
         *
         *      No parameters.
         */

        case XM_INSERTDATA:
        {
            ULONG ulError = 0;
            PLVMDATA pData = (PLVMDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

            fprintf(G_LogFile, __FUNCTION__ ": processing XM_INSERTDATA\n"
                       );

            // clean up if not first post
            FreeLVMData(pData,
                        FALSE);         // do not free engine yet

            if (!(ulError = thrRunSync(WinQueryAnchorBlock(hwndClient),
                                       fntLoadLVMData,
                                       "loadLVMData",
                                       (ULONG)pData)))
            {
                fprintf(G_LogFile, __FUNCTION__ ": calling SetControlsData\n");

                // update controls with the data
                SetControlsData(hwndClient);

                fprintf(G_LogFile, __FUNCTION__ ": calling PositionChartsInClient\n");

                // position the controls in the client
                PositionChartsInClient(hwndClient);

                WinSetWindowText(G_hwndStatusBar,
                                 "xlvm is ready.");

                fprintf(G_LogFile, __FUNCTION__ ": done processing XM_INSERTDATA\n");
            }
            else
            {
                WinSetWindowText(G_hwndStatusBar,
                                 "An error occurred loading the LVM data.");
            }
        }
        break;

        /*
         *@@ XM_LVMERROR:
         *      mp1 has error code
         *      mp2 has context
         */

        case XM_LVMERROR:
        {
            XSTRING str;
            xstrInit(&str, 0);
            xstrPrintf(&str,
                    "An error returned in the LVM engine. %s returned error code %d (%s).",
                    mp2,
                    mp1,
                    DescribeLVMError((ULONG)mp1));
            MessageBox(hwndClient,
                       &str,
                       MB_OK);
            xstrClear(&str);
        }
        break;

        /*
         * WM_PAINT:
         *
         */

        case WM_PAINT:
        {
            fprintf(G_LogFile, __FUNCTION__ ": processing WM_PAINT\n");

            RECTL rclClient;
            HPS hps = WinBeginPaint(hwndClient, NULLHANDLE, NULL);
            WinQueryWindowRect(hwndClient, &rclClient);
            gpihSwitchToRGB(hps);
            WinFillRect(hps,
                        &rclClient,
                        WinQuerySysColor(HWND_DESKTOP,
                                         SYSCLR_APPWORKSPACE,
                                         0));
            WinEndPaint(hps);

            fprintf(G_LogFile, __FUNCTION__ ": done processing WM_PAINT\n");
        }
        break;

        case WM_SIZE:
            fprintf(G_LogFile, __FUNCTION__ ": processing WM_SIZE\n");
            PositionChartsInClient(hwndClient);
            fprintf(G_LogFile, __FUNCTION__ ": done processing WM_SIZE\n");
        break;

        /*
         * WM_COMMAND:
         *
         */

        case WM_COMMAND:
            fprintf(G_LogFile, __FUNCTION__ ": processing WM_COMMAND 0x%lX\n", SHORT1FROMMP(mp1));
            ClientCommand(hwndClient, mp1);
            fprintf(G_LogFile, __FUNCTION__ ": done processing WM_COMMAND 0x%lX\n", SHORT1FROMMP(mp1));
        break;

        /*
         * WM_CONTROL:
         *
         */

        case WM_CONTROL:
        {
            fprintf(G_LogFile, __FUNCTION__ ": processing WM_CONTROL 0x%lX\n", SHORT1FROMMP(mp1));

            SHORT usid = SHORT1FROMMP(mp1),
                  uscode = SHORT2FROMMP(mp1);
            if (usid >= 1000)        // chart controls have 1000 + drive no.
            {
                switch (uscode)
                {
                    case CHTN_EMPHASISCHANGED:
                    case CHTN_CONTEXTMENU:
                    case CHTN_SETFOCUS:  // V0.9.20 (2002-07-17) [pr]
                    {
                        PLVMDATA pData = (PLVMDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

                        PLVMDRIVEDATA pDriveData
                                = (PLVMDRIVEDATA)lstItemFromIndex(&pData->llDriveData,
                                                                  usid - 1000);
                        if (pDriveData)
                        {
                            // now we got the drive where a new volume
                            // was selected: get the volume then from
                            // the chart control data given to us
                            PEMPHASISNOTIFY pen = (PEMPHASISNOTIFY)mp2;
                            if (pen)
                            {
                                if (   (   (uscode == CHTN_EMPHASISCHANGED)
                                        && (pen->ulEmphasis == 0)      // selection changed
                                       )
                                    || (uscode == CHTN_SETFOCUS)  // V0.9.20 (2002-07-17) [pr]
                                   )
                                {
                                    if (pen->lIndex == -1)
                                        // deselected:
                                        WinSetWindowText(G_hwndStatusBar, "");
                                    else if (pen->lIndex < pDriveData->PartitionsArray.Count)
                                    {
                                        // selection changed:
                                        Partition_Information_Record *pBarItem
                                            = &pDriveData->PartitionsArray.Partition_Array[pen->lIndex];
                                        CHAR sz[1000] = "?";
                                        GetVolumeDescription(pData, sz, pBarItem);
                                        WinSetWindowText(G_hwndStatusBar, sz);
                                    }
                                }
                                else
                                    if (    (pen->lIndex < pDriveData->PartitionsArray.Count)
                                         && (uscode == CHTN_CONTEXTMENU)
                                       )
                                    {
                                        Partition_Information_Record *pBarItem
                                            = &pDriveData->PartitionsArray.Partition_Array[pen->lIndex];
                                        {
                                            ShowContextMenu(pData,
                                                            pen,
                                                            pBarItem);
                                        }
                                    }
                            }
                        }
                    }
                    break;

                } // end switch
            }

            fprintf(G_LogFile, __FUNCTION__ ": done processing WM_CONTROL 0x%lX\n", SHORT1FROMMP(mp1));
        }
        break;

        /*
         * WM_MENUEND:
         *      remove source emphasis when context menu goes
         *      away.
         */

        case WM_MENUEND:
            if (    (G_hwndMenuShowing)
                 && ((HWND)mp2 == G_hwndMenuShowing)
               )
            {
                WinSendMsg(G_CurrentEmphasis.hwndSource,
                           CHTM_SETEMPHASIS,
                           (MPARAM)1,           // source emphasis
                           (MPARAM)-1);         // remove
            }
        break;

        /*
         * WM_CLOSE:
         *
         */

        case WM_CLOSE:
        {
            PLVMDATA pData = (PLVMDATA)WinQueryWindowPtr(hwndClient, QWL_USER);

            fprintf(G_LogFile, __FUNCTION__ ": processing WM_CLOSE\n");
            BOOL fClose = FALSE;

            if (!pData->fDirty)
                fClose = TRUE;
            else
            {
                // dirty:
                fClose = ConfirmCommit(hwndClient);
            }

            if (fClose)
            {
                winhSaveWindowPos(WinQueryWindow(hwndClient, QW_OWNER),
                                  HINI_USER,
                                  INIAPP,
                                  INIKEY_MAINWINPOS);
                WinSetWindowText(G_hwndStatusBar,
                                 "Cleaning up...");

                fprintf(G_LogFile, __FUNCTION__ ":   posting WM_QUIT\n");

                WinPostMsg(hwndClient, WM_QUIT, 0, 0);

                FreeLVMData(pData,
                            TRUE);      // close engine
            }

            fprintf(G_LogFile, __FUNCTION__ ": done processing WM_CLOSE\n");
        }
        break;

        default:
            mrc = WinDefWindowProc(hwndClient, msg, mp1, mp2);
    }

    return (mrc);
}

/* ******************************************************************
 *
 *   Frame window proc
 *
 ********************************************************************/

/*
 *@@ winhCreateStatusBar:
 *
 *@@added V0.9.5 (2000-08-13) [umoeller]
 */

HWND winhCreateStatusBar(HWND hwndFrame,
                         HWND hwndOwner,
                         USHORT usID,
                         PCSZ pcszFont,
                         LONG lColor)
{
    // create status bar
    HWND        hwndReturn = NULLHANDLE;
    PPRESPARAMS ppp = NULL;
    winhStorePresParam(&ppp, PP_FONTNAMESIZE, strlen(pcszFont)+1, (PVOID)pcszFont);
    lColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0);
    winhStorePresParam(&ppp, PP_BACKGROUNDCOLOR, sizeof(lColor), &lColor);
    lColor = CLR_BLACK;
    winhStorePresParam(&ppp, PP_FOREGROUNDCOLOR, sizeof(lColor), &lColor);
    hwndReturn = WinCreateWindow(hwndFrame,
                                 WC_STATIC,
                                 "Loading LVM data, please wait...",
                                 SS_TEXT | DT_VCENTER | WS_VISIBLE,
                                 0, 0, 0, 0,
                                 hwndOwner,
                                 HWND_TOP,
                                 usID,
                                 NULL,
                                 ppp);
    free(ppp);
    return (hwndReturn);
}

/*
 *@@ winh_fnwpFrameWithStatusBar:
 *      subclassed frame window proc.
 *
 *@@added V0.9.5 (2000-08-13) [umoeller]
 */

MRESULT EXPENTRY winh_fnwpFrameWithStatusBar(HWND hwndFrame, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_QUERYFRAMECTLCOUNT:
        {
            // query the standard frame controls count
            ULONG ulrc = (ULONG)(G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2));

            // if we have a status bar, increment the count
            ulrc++;

            mrc = (MPARAM)ulrc;
        }
        break;

        case WM_FORMATFRAME:
        {
            //  query the number of standard frame controls
            ULONG ulCount = (ULONG)(G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2));

            // we have a status bar:
            // format the frame
            ULONG       ul;
            PSWP        swpArr = (PSWP)mp1;

            for (ul = 0; ul < ulCount; ul++)
            {
                if (WinQueryWindowUShort( swpArr[ul].hwnd, QWS_ID ) == 0x8008 )
                                                                 // FID_CLIENT
                {
                    POINTL      ptlBorderSizes;
                    ULONG       ulStatusBarHeight = 20;
                    WinSendMsg(hwndFrame,
                               WM_QUERYBORDERSIZE,
                               (MPARAM)&ptlBorderSizes,
                               0);

                    // first initialize the _new_ SWP for the status bar.
                    // Since the SWP array for the std frame controls is
                    // zero-based, and the standard frame controls occupy
                    // indices 0 thru ulCount-1 (where ulCount is the total
                    // count), we use ulCount for our static text control.
                    swpArr[ulCount].fl = SWP_MOVE | SWP_SIZE | SWP_NOADJUST | SWP_ZORDER;
                    swpArr[ulCount].x  = ptlBorderSizes.x;
                    swpArr[ulCount].y  = ptlBorderSizes.y;
                    swpArr[ulCount].cx = swpArr[ul].cx;  // same as cnr's width
                    swpArr[ulCount].cy = ulStatusBarHeight;
                    swpArr[ulCount].hwndInsertBehind = HWND_BOTTOM; // HWND_TOP;
                    swpArr[ulCount].hwnd = G_hwndStatusBar;

                    // adjust the origin and height of the container to
                    // accomodate our static text control
                    swpArr[ul].y  += swpArr[ulCount].cy;
                    swpArr[ul].cy -= swpArr[ulCount].cy;
                }
            }

            // increment the number of frame controls
            // to include our status bar
            mrc = (MRESULT)(ulCount + 1);
        }
        break;

        case WM_CALCFRAMERECT:
            mrc = G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2);

            // we have a status bar: calculate its rectangle
            // CalcFrameRect(mp1, mp2);
        break;

        default:
            mrc = G_fnwpFrameOrig(hwndFrame, msg, mp1, mp2);
    }

    return (mrc);
}

/* ******************************************************************
 *
 *   main
 *
 ********************************************************************/

#define WC_XLVMCLIENT "xlvmClientClass"

/*
 *@@ main:
 *
 */

int main(int argc, char* argv[])
{
    HAB         hab;
    HMQ         hmq;
    QMSG        qmsg;

    if (!(hab = WinInitialize(0)))
        return FALSE;

    if (!(hmq = WinCreateMsgQueue(hab, 0)))
        return FALSE;

    winhInitGlobals();      // V1.0.1 (2002-11-30) [umoeller]

    DosError(FERR_DISABLEHARDERR | FERR_ENABLEEXCEPTION);

    nlsQueryCountrySettings(&G_CountrySettings);

    TRY_LOUD(excpt1)
    {
        const char *p = strrchr(argv[0], '\\');
        if (p)
        {
            char sz[300];
            strhncpy0(sz, argv[0], p - argv[0]);
            strcat(sz, "\\xlvm.log");
            G_LogFile = fopen(sz, "w");

            fprintf(G_LogFile, "xlvm log\n");
        }

        G_hptrDrive = WinLoadPointer(HWND_DESKTOP,
                                     NULLHANDLE,
                                     IDP_DRIVE);

        G_hptrPartition = WinLoadPointer(HWND_DESKTOP,
                                         NULLHANDLE,
                                         IDP_PARTITION);

        G_hwndWhitespaceMenu = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_WHITESPACE);
        G_hwndFreeSpaceMenu = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_FREESPACE);
        G_hwndPartitionMenu = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_PARTITION);
        G_hwndVolumeMenu = WinLoadMenu(HWND_OBJECT, NULLHANDLE, IDM_VOLUME);

        // register client
        if (!WinRegisterClass(hab,
                              WC_XLVMCLIENT,
                              fnwpClient,
                              CS_SIZEREDRAW | CS_CLIPCHILDREN,
                              4))
            fprintf(G_LogFile, "WinRegisterClass failed.\n");
        else
        {
            ctlRegisterSeparatorLine(hab);

            // create frame and container
            HWND    hwndClient = NULLHANDLE,
                    hwndMain = winhCreateStdWindow(HWND_DESKTOP,
                                                   0,
                                                   FCF_TITLEBAR
                                                      | FCF_SYSMENU
                                                      | FCF_MINMAX
                                                      | FCF_SIZEBORDER
                                                      | FCF_ICON
                                                      | FCF_MENU
                                                      | FCF_TASKLIST,
                                                   0,
                                                   APPTITLE,      // xmlview
                                                   1,             // icon resource
                                                   WC_XLVMCLIENT, // client class
                                                   WS_VISIBLE,
                                                   0,
                                                   NULL,
                                                   &hwndClient);

            fprintf(G_LogFile, "G_hwndMain is 0x%lX, G_hwndClient is 0x%lX\n",
                        hwndMain, hwndClient);

            if ((hwndMain) && (hwndClient))
            {
                // create status bar as child of the frame
                G_hwndStatusBar = winhCreateStatusBar(hwndMain,
                                                      hwndClient,
                                                      0,
                                                      G_pcszDefaultFont,
                                                      CLR_BLACK);
                fprintf(G_LogFile, "G_hwndStatusBar is 0x%lX\n",
                            G_hwndStatusBar);

                // subclass frame for supporting status bar and msgs
                G_fnwpFrameOrig = WinSubclassWindow(hwndMain,
                                                    winh_fnwpFrameWithStatusBar);

                fprintf(G_LogFile, "G_fnwpFrameOrig is 0x%lX\n",
                            G_fnwpFrameOrig);

                winhSetWindowFont(hwndClient, G_pcszDefaultFont);

                RefreshViewMenu(hwndMain);

                if (!winhRestoreWindowPos(hwndMain,
                                          HINI_USER,
                                          INIAPP,
                                          INIKEY_MAINWINPOS,
                                          SWP_SHOW | SWP_ACTIVATE | SWP_MOVE | SWP_SIZE))
                {
                    fprintf(G_LogFile, "restore winpos failed, setting explicit window pos\n"
                                );
                    WinSetWindowPos(hwndMain,
                                    HWND_TOP,
                                    10, 10, 500, 500,
                                    SWP_SHOW | SWP_ACTIVATE | SWP_MOVE | SWP_SIZE);
                }

                fprintf(G_LogFile, "posting XM_INSERTDATA\n"
                           );

                fprintf(G_LogFile, __FUNCTION__ ": entering msg loop\n"
                           );

                //  standard PM message loop
                while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
                {
                    WinDispatchMsg(hab, &qmsg);
                }

                fprintf(G_LogFile, __FUNCTION__ ": leaving msg loop\n"
                           );

                WinDestroyWindow(hwndMain);
            }
        }
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    // clean up on the way out
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return (0);
}

