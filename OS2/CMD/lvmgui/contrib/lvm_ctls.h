/*****************************************************************************
 ** LVMPM - lvm_ctls.h                                                      **
 *****************************************************************************
 * Custom window controls used for graphical display of LVM information.     *
 *                                                                           *
 * WC_LVMDISKS (disk-list control)                                           *
 *  - A scrollable list of WC_DISKVIEW controls.                             *
 *                                                                           *
 * WC_DISKVIEW (disk-view control)                                           *
 *  - Represents a single physical disk drive, showing an icon, name and     *
 *    size labels, and a graphic showing the partition layout.  The graphic  *
 *    consists of zero or more WC_PARTITIONVIEW controls.                    *
 *                                                                           *
 * WC_PARTITIONVIEW (partition-view control)                                 *
 *  - Represents a single partition, with a name and various visual          *
 *    indicators.  May optionally display a drive letter in an overlay box   *
 *    (designed to show when a partition belongs to a visible volume).       *
 *                                                                           *
 * WC_VOLUMEINFO (volume details panel)                                      *
 *  - Standalone control (not part of the disk/partition view hierarchy of   *
 *    those listed above) designed to show information about a logical       *
 *    volume.  Contains an icon, fields for drive letter, name, volume type  *
 *    and size, device type, and a container listing all member partitions.  *
 *                                                                           *
 * This file also includes several utility functions which are used by these *
 * controls; these have public prototypes as they are generally useful.      *
 *                                                                           *
 *****************************************************************************
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify it *
 *   under the terms of the GNU General Public License as published by the   *
 *   Free Software Foundation; either version 2 of the License, or (at your  *
 *   option) any later version.                                              *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License along *
 *   with this program; if not, write to the Free Software Foundation, Inc., *
 *   59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                 *
 *****************************************************************************/

#define INCL_GPI
#define INCL_WIN
#ifndef OS2_INCLUDED
  #include <os2.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// uncomment to keep label string pointers in VICTLDATA instead of VIVOLUMEINFO
#define VICTLDATA_STRINGS

// ----------------------------------------------------------------------------
// LVM INTERFACE DEFINITIONS

#ifndef LVM_INTERFACE_H_INCLUDED
  #define DISK_NAME_SIZE        20
  #define FILESYSTEM_NAME_SIZE  20
  #define PARTITION_NAME_SIZE   20
  #define VOLUME_NAME_SIZE      20
#endif


// ----------------------------------------------------------------------------
// CONSTANTS

// Class names for our custom controls
#define WC_PARTITIONVIEW    "LVMPartitionView"  // partition panel
#define WC_DISKVIEW         "LVMDiskView"       // disk panel
#define WC_LVMDISKS         "LVMDiskList"       // scrollable list of disk panels
#define WC_VOLUMEINFO       "LVMVolumeInfo"     // volume details panel

// Values for bType field of PVCTLDATA
#define LPV_TYPE_FREE       0       // usable free space
#define LPV_TYPE_PRIMARY    1       // primary partition
#define LPV_TYPE_LOGICAL    2       // logical partition
#define LPV_TYPE_BOOTMGR    3       // active "Boot Manager" primary partition
#define LPV_TYPE_UNUSABLE   9       // unusable (orphaned) free space

// Partition emphasis styles used by LDM_SETEMPHASIS & LPM_SETEMPHASIS
#define LPV_FS_SELECTED     0x01    // used to draw selection cursor
#define LPV_FS_ACTIVE       0x02    // used to indicate "active" emphasis
#define LPV_FS_CONTEXT      0x10    // used to indicate context-menu emphasis

// Disk emphasis styles used by LDM_SETEMPHASIS
#define LDV_FS_SELECTED     0x0100  // used to draw selection cursor
#define LDV_FS_FOCUS        0x0200  // used to draw keyboard-focus border
#define LDV_FS_CONTEXT      0x1000  // used to indicate context-menu emphasis

// Disk styles used by LDM_SETSTYLE
#define LDS_FS_UNIFORM      0x01    // show all partitions with uniform width

// Disk list styles used by LLM_SETSTYLE
#define LLD_FS_TITLE        0x01    // show container-style list title

// Values for fsStyle field of VICTLDATA
#define VIV_FS_3DSEP        0x01    // use a 3D-style horizontal separator

// String lengths
#define SIZE_TEXT_LIMIT     50      // length of the disk-size text


// ----------------------------------------------------------------------------
// CUSTOM MESSAGES
//

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// For WC_DISKVIEW
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ............................................................................
// LDM_SETPARTITIONS
//  - mp1 (ULONG)     : Number of partitions (note: this is a ULONG in order to
//                      facilitate direct passing of the 32-bit partition count
//                      from LVM; however, it has a maximum value of 65536)
//  - mp2 (PPVCTLDATA): Array of partition control data structures
//  Returns (BOOL) TRUE on success, FALSE on failure
//
// Set the array of partitions on this disk.  This replaces any previously
// defined partitions.  If mp1 is zero, all existing partitions are removed
// (and mp2 is ignored).  If mp1 is non-zero, mp2 must be a pointer to one or
// more PVTCTLDATA structures (these can be freed or un-scoped afterwards).
//
// The array entries SHOULD be ordered according to partition number (so that
// entry 0 is for partition 1, entry 1 is for partition 2, and so on).  There
// is no loss of functionality if this requirement is not adhered to, but the
// partitions will be drawn in the disk layout graph in the order in which
// they are provided here, and it is obviously desirable that they should
// appear in their actual physical order.
//
// NOTE: There is a theoretical limit of 65536 partitions per disk.
// ............................................................................

#define LDM_SETPARTITIONS           (WM_USER + 100)


// ............................................................................
// LDM_SETDISKICON
//  - mp1 (HPOINTER): Handle of the icon to use
//  - mp2 (ULONG)   : Unused, should be 0
//  Returns 0
//
// Set the icon used for this disk.  mp1 should be a handle to an icon as
// obtained from WinLoadIcon or the equivalent.
// ............................................................................

#define LDM_SETDISKICON             (WM_USER + 101)


// ............................................................................
// LDM_SETEMPHASIS
//  - mp1 (HWND)  : Handle of the partition control whose emphasis is to be set
//                  (may be 0, see below).
//  - mp2 (BOOL)  : If TRUE: set given emphasis type(s).  If mp1 is 0, the
//                    emphasis will be set on the FIRST partition.
//                  If FALSE: clear the given emphasis type(s).  If mp1 is 0,
//                    the emphasis will be cleared on ALL partitions.
//        (USHORT): Emphasis type(s) to set.  For partitions these are:
//                   LPV_FS_SELECTED  selection/focus emphasis (dotted border)
//                   LPV_FS_ACTIVE    "active" emphasis (diagonal hatching)
//                   LPV_FS_CONTEXT   context-menu emphasis (dashed border)
//                  For disks, the available emphasis types are:
//                   LDV_FS_SELECTED  selection/focus emphasis (highlight)
//                   LDV_FS_FOCUS     input-focus emphasis (dotted border)
//                   LDV_FS_CONTEXT   context-menu emphasis (dashed border)
// Returns (BOOL) TRUE on success, FALSE on failure
//
// Set or clear a particular emphasis type for a WC_DISKVIEW control and/or one
// of the WC_PARTITIONVIEW children it owns.  If mp1 is non-zero it is taken to
// be the HWND of the WC_PARTITIONVIEW whose emphasis is to be updated.  The
// first USHORT (BOOL) of mp2 indicates whether the emphasis will be set (TRUE)
// or cleared/unset (FALSE).  The second USHORT of mp2 contains the emphasis
// flags being updated; the low-order byte contains partition-specific flags,
// and the high-order byte contains disk-specific flags.
//
// If mp1 is non-zero it is taken to be the HWND of a WC_PARTITIONVIEW control
// belonging to the disk whose emphasis is to be updated.  An mp1 of 0
// (NULLHANDLE) results in the following behaviour:
// - When setting emphasis flags, any partition-specific emphasis flags which
//   are specified in mp2 will be set on the first WC_PARTITIONVIEW control
//   defined for the disk.
// - When clearing emphasis flags, any partition-specific emphasis flags which
//   are specified in mp2 will be cleared from all WC_PARTITIONVIEW controls
//   defined for the disk.
//
// NOTES:
// - Only one partition within a disk may have selection emphasis at one time.
// - If a disk does not have selection emphasis, none of its partitions may.
// - When a disk which did not previously have selection emphasis is given it,
//   one of its partitions will also be given selection emphasis (automatically
//   if necessary).
// - Only one disk or partition can have context-menu emphasis at one time;
//   this is because we assume that only one context menu can be open at once.
//
// As a consequence, the following side-effects apply:
// - Setting LDV_FS_SELECTED on the disk without specifying it for any
//   partition will cause LPV_FS_SELECTED to be set on the FIRST partition on
//   the disk (except in the case where the disk already had selected emphasis,
//   in which case it is assumed that no change is necessary).
// - Clearing LDV_FS_SELECTED from a disk will automatically clear
//   LPV_FS_SELECTED from ALL partitions owned by that disk.
// - Setting LPV_FS_SELECTED on any partition will clear it from all OTHER
//   partitions owned by that disk.
// - Setting context-menu emphasis (LDV_FS_CONTEXT/LPV_FS_CONTEXT) on any window
//   (the disk or any of its child partitions) will clear it from all others.
// ............................................................................

#define LDM_SETEMPHASIS         (WM_USER + 102)


// ............................................................................
// LDM_GETEMPHASIS
//  - mp1 (HWND): If NULLHANDLE, return the emphasis flags of the disk control.
//                Otherwise, return the emphasis flags of the WC_PARTITIONVIEW
//                control whose HWND this corresponds to.
//  - mp2:        Unused, should be 0.
//  Returns (USHORT) the emphasis flags active for the disk or partition.
//                For partitions these are:
//                 LPV_FS_SELECTED  selection/focus emphasis (dotted border)
//                 LPV_FS_ACTIVE    "active" emphasis (diagonal hatching)
//                 LPV_FS_CONTEXT   context-menu emphasis (dashed border)
//                For disks, the emphasis types are:
//                 LDV_FS_SELECTED  selection/focus emphasis (highlight)
//                 LDV_FS_FOCUS     input-focus emphasis (dotted border)
//                 LDV_FS_CONTEXT   context-menu emphasis (dashed border)
// ............................................................................

#define LDM_GETEMPHASIS       (WM_USER + 103)


// ............................................................................
// LDM_QUERYPARTITIONHWND
//  - mp1 (CARDINAL32): Index of the partition within the disk's internal array
//                        (from 0), or the LVM partition handle
//  - mp2 (BOOL)      : TRUE if mp1 contains the partition index
//                      FALSE if  mp1 contains the LVM partition handle
//  Returns (HWND) the window handle of the corresponding WC_PARTITIONVIEW
//
// Queries the HWND of a WC_PARTITIONVIEW control owned by the disk.  If mp2
// is TRUE, mp1 is taken as the index number of the partition entry within the
// WC_DISKVIEW's internal array (representing the order in which the partitions
// were defined in the call to LDM_SETPARTITONS).  If mp2 is FALSE, then mp1 is
// taken as the unique LVM engine handle for the desired partition.
// ............................................................................

#define LDM_QUERYPARTITIONHWND  (WM_USER + 104)


// ............................................................................
// LDM_QUERYPARTITIONEMPHASIS
//  - mp1 (HWND):   HWND of the WC_PARTITIONVIEW control to start searching
//                    after, or 0 to search from the first partition.
//  - mp2 (USHORT): Emphasis flags to search for
//  Returns (HWND) the window handle of the first matching WC_PARTITIONVIEW
//
// Searches the disk control for a WC_PARTITIONVIEW child control with (all of)
// the specified emphasis type(s).
// ............................................................................

#define LDM_QUERYPARTITIONEMPHASIS  (WM_USER + 105)


// ............................................................................
// LDM_QUERYLASTSELECTED
//   - mp1 (BOOL): TRUE to return the partition's HWND
//                 FALSE to return the partition index number
//   - mp2       : Unused, should be 0
// Returns either (HWND) the window handle or (USHORT) the number of the most
// recently selected partition
//
// Queries the disk control for the partition which most recently had selection
// emphasis.  Normally, this is whichever partition currently has selection
// emphasis; however, the selection emphasis can be cleared or lost even though
// the disk control retains a record of the selection state.  This message
// allows the most-recently selected partition to be determined even if the
// selection emphasis has been lost.
// ............................................................................

#define LDM_QUERYLASTSELECTED       (WM_USER + 106)


// ............................................................................
// LDM_QUERYPARTITIONS
//  - mp1 : Unused, should be 0
//  - mp2 : Unused, should be 0
// Returns (USHORT) the number of WC_PARTITIONVIEW children owned by the disk.
// ............................................................................

#define LDM_QUERYPARTITIONS         (WM_USER + 107)


// ............................................................................
// LDM_SETSTYLE
//  - mp1 (USHORT) : New style flags
//  - mp2          : Unused, should be 0
//  Returns 0.
//
// Changes the current style flags for the disk view control.  The new style
// mask (in mp1) replaces the old one.
// Currently, the only supported style flag is LDS_FS_UNIFORM
// ............................................................................

#define LDM_SETSTYLE                (WM_USER + 108)


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// For WC_PARTITIONVIEW
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ............................................................................
// LPM_SETEMPHASIS
//  - mp1 (BOOL)  : TRUE to set given emphasis type, FALSE to unset it
//  - mp2 (USHORT): Emphasis type to set, one of:
//                   LPV_FS_SELECTED  selection/focus emphasis (dotted border)
//                   LPV_FS_ACTIVE    "active" emphasis (diagonal hatching)
//                   LPV_FS_CONTEXT   context-menu emphasis (dashed border)
//  Returns (BOOL) TRUE on success, FALSE on failure
//
// Set a particular emphasis type on a WC_PARTITIONVIEW control.
// ............................................................................

#define LPM_SETEMPHASIS         (WM_USER + 110)


// ............................................................................
// LPM_GETEMPHASIS
//  - mp1: Unused, should be 0
//  - mp2: Unused, should be 0
//  Returns (USHORT) the emphasis flags active for the partition.  Values are:
//                 LPV_FS_SELECTED  selection/focus emphasis (dotted border)
//                 LPV_FS_ACTIVE    "active" emphasis (diagonal hatching)
//                 LPV_FS_CONTEXT   context-menu emphasis (dashed border)
// ............................................................................

#define LPM_GETEMPHASIS       (WM_USER + 111)


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// For WC_LVMDISKS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ............................................................................
// LLM_SETDISKS
//  - mp1 (ULONG)     : Number of disks (note: this is a ULONG in order to
//                      facilitate direct passing of the 32-bit disk count from
//                      LVM; however, it has a maximum value of 65536)
//  - mp2 (PDVCTLDATA): Array of disk control data structures
//  Returns (BOOL) TRUE on success, FALSE on failure
//
// Set the array of disks for the disk list.  This replaces any previously
// defined disks.  If mp1 is zero, all existing disks are removed (and mp2 is
// ignored).  If mp1 is non-zero, mp2 must be a pointer to one or more
// DVTCTLDATA structures (these can be freed or un-scoped afterwards).
//
// NOTE: There is a theoretical maximum of 65536 disks.
// ............................................................................

#define LLM_SETDISKS            (WM_USER + 120)


// ............................................................................
// LLM_QUERYDISKHWND
//  - mp1 (see below): Index of the disk within the list's internal array, or
//                       the LVM disk handle
//  - mp2 (BOOL)     : TRUE if mp1 contains the disk index
//                     FALSE if mp1 contains the LVM disk handle
//  Returns (HWND) the window handle of the corresponding WC_DISKVIEW
//
// Queries the HWND of a WC_DISKVIEW control in the disk list.  If mp2 is TRUE,
// mp1 is taken as the index number (CARDINAL32) of the disk entry within the
// WC_LVMDISKS control's internal array (representing the order in which the
// disks were defined in the call to LLM_SETDISKS).  If mp2 is FALSE, then mp1
// is taken as the unique LVM engine handle (ADDRESS) for the desired disk.
// ............................................................................

#define LLM_QUERYDISKHWND       (WM_USER + 121)


// ............................................................................
// LLM_QUERYDISKEMPHASIS
//  - mp1 (HWND):   HWND of the disk control to start searching after, or 0 to
//                    search from the first disk
//  - mp2 (USHORT): Emphasis flags to search for
//  Returns (HWND) the window handle of the first matching WC_DISKVIEW
//
// Searches the disk list for a WC_DISKVIEW child with (all of) the specified
// emphasis type(s).
// ............................................................................

#define LLM_QUERYDISKEMPHASIS   (WM_USER + 122)


// ............................................................................
// LLM_SETDISKEMPHASIS
//  - mp1 (HWND)  : Handle of the WC_DISKVIEW control whose emphasis is to be
//                    set.
//  - mp2 (BOOL)  : If TRUE: set given emphasis type(s)
//                  If FALSE: clear the given emphasis type(s)
//        (USHORT): Emphasis type(s) to set; available values are:
//                   LDV_FS_SELECTED  selection/focus emphasis (highlight)
//                   LDV_FS_FOCUS     input-focus emphasis (dotted border)
//                   LDV_FS_CONTEXT   context-menu emphasis (dashed border)
// Returns (BOOL) TRUE on success, FALSE on failure
//
// Set or clear a particular emphasis type for a WC_DISKVIEW control owned by
// the WC_LVMDISKS list.  This message does not allow emphasis flags to be
// updated on any WC_PARTITIONVIEW controls (except via the side effects
// described below); if such is desired, a LDM_SETEMPHASIS message must be
// sent directly to the applicable disk (WC_DISKVIEW) control.
//
// NOTES:
// - Only one disk owned by the list may have selection emphasis at one time.
//   Setting LDV_FS_SELECTED on any WC_DISKVIEW will clear the same flag from
//   all other WC_DISKVIEW children owned by the disk list.
// - By the same token, only one disk owned by the list may have focus emphasis
//   at one time. Setting LDV_FS_FOCUS on any WC_DISKVIEW will clear the same
//   flag from all other WC_DISKVIEW children owned by the disk list.
// - Similarly, only one disk may have context-menu emphasis at one time.
//   Setting LDV_FS_CONTEXT on any WC_DISKVIEW will clear the same flag from
//   all other WC_DISKVIEW children owned by the disk list (and also from any
//   WC_PARTITIONVIEW children of that or any other disk).
// The following additional side-effects are caused by the LDM_SETEMPHASIS
// message which is used internally to update the disk emphasis:
// - If a WC_DISKVIEW which did not previously have selection emphasis has the
//   LDV_FS_SELECTED flag set, the first WC_PARTITIONVIEW child of that disk
//   (if any) will automatically have LPV_FS_SELECTED set.
// - Clearing LDV_FS_SELECTED from a WC_DISKVIEW will automatically clear the
//   LPV_FS_SELECTED flag from all WC_PARTITIONVIEW children of that disk.
// ............................................................................

#define LLM_SETDISKEMPHASIS     (WM_USER + 123)


// ............................................................................
// LLM_GETPARTITION
//  - mp1 (CARDINAL32) : LVM partition handle
//  - mp2 (CARDINAL32) : LVM disk handle (optional)
//  Returns (HWND) the window handle of the corresponding WC_PARTITIONVIEW
//
// Queries the HWND of the WC_PARTITIONVIEW control whose LVM handle corresponds
// to that specified in mp1.  If an LVM disk handle is also specified in mp2,
// only the WC_DISKVIEW control with that handle is searched; if mp2 is 0, all
// WC_DISKVIEW controls owned by the disk list are searched.
// ............................................................................

#define LLM_GETPARTITION        (WM_USER + 124)


// ............................................................................
// LLM_SETSTYLE
//  - mp1 (USHORT) : New style flags
//  - mp2          : Unused, should be 0
//  Returns 0.
//
// Changes the current style flags for the disk list control.  The new style
// mask (in mp1) replaces the old one.
// Currently, the only supported style flag is LLD_FS_TITLE.
// ............................................................................

#define LLM_SETSTYLE            (WM_USER + 125)


// ............................................................................
// LLM_SETTITLE
//  - mp1 (PSZ) : Title string
//  - mp2       : Unused, should be 0
//  Returns 0.
//
// Sets or changes the title string of the disk list control.  The string (in
// mp1) is copied so that the original can be freed after sending this message.
// ............................................................................

#define LLM_SETTITLE            (WM_USER + 126)


// ............................................................................
// LLM_QUERYDISKS
//  - mp1 : Unused, should be 0
//  - mp2 : Unused, should be 0
// Returns (USHORT) the number of WC_DISKVIEW children owned by the disk list.
// ............................................................................

#define LLM_QUERYDISKS          (WM_USER + 127)



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// For WC_VOLUMEINFO
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ............................................................................
// VIM_SETINFO
//  - mp1 (PVIVOLUMEINFO): Pointer to volume information
//  - mp2                : Unused, should be 0.
//  Returns (BOOL) TRUE on success, FALSE on failure
//
// Set the current volume information to be displayed in the volume information
// panel.  This replaces any previously volume information.  mp1 is a pointer
// to a VIVOLUMEINFO structure containing the volume information.  Note that
// the hIcon field of the VIVOLUMEINFO structure must not be freed, as the
// volume information control stores it as a pointer.
// ............................................................................

#define VIM_SETINFO             (WM_USER + 130)


// ............................................................................
// VIM_SETSTYLE
//  - mp1 (USHORT) : New style flags
//  - mp2          : Unused, should be 0
//  Returns 0.
//
// Changes the current style flags for the volume information panel.  The new
// style mask (in mp1) replaces the old one.
// Currently, the only supported style flag is VIV_FS_3DSEP.
// ............................................................................

#define VIM_SETSTYLE            (WM_USER + 131)


// ............................................................................
// VIM_GETCONTAINERHWND
//  - mp1 : Unused, should be 0
//  - mp2 : Unused, should be 0
//  Returns (HWND) the window handle of the partition container control
//
// Returns the HWND of the partition container owned by the volume information
// panel.
// ............................................................................

#define VIM_GETCONTAINERHWND    (WM_USER + 132)


// ----------------------------------------------------------------------------
// NOTIFICATION CODES
//

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sent by WC_DISKVIEW
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ............................................................................
// LDN_SELECT
//   Either the WC_DISKVIEW control or one of its WC_PARTITIONVIEW children
//   was selected.  mp2 is a pointer to a DISKNOTIFY structure.
// ............................................................................
#define LDN_SELECT          100

// ............................................................................
// LDN_CONTEXTMENU
//   Either the WC_DISKVIEW control or one of its WC_PARTITIONVIEW children
//   requested a popup menu.  mp2 is a pointer to a DISKNOTIFY structure.
// ............................................................................
#define LDN_CONTEXTMENU     101


// ............................................................................
// LDN_EMPHASIS **DEPRECATED
//   Either the WC_DISKVIEW control or one of its WC_PARTITIONVIEW children
//   had an emphasis flag changed.  mp2 is a pointer to a DISKNOTIFY
//   structure.
// ............................................................................
//#define LDN_EMPHASIS          100


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sent by WC_PARTITIONVIEW
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ............................................................................
// LPN_SELECT
//     The WC_PARTITIONVIEW control was selected.  mp2 will be the HWND of the
//     control.
// ............................................................................
#define LPN_SELECT          110


// ............................................................................
// LPN_CONTEXTMENU
//     The WC_PARTITIONVIEW control requested a popup menu.  mp2 is a pointer
//     to a DISKNOTIFY structure.
// ............................................................................
#define LPN_CONTEXTMENU     111


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Sent by WC_LVMDISKS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ............................................................................
// LLN_EMPHASIS
//     An emphasis change occured in a WC_DISKVIEW or a child WC_PARTITIONVIEW.
//     mp2 is a pointer to a DISKNOTIFY structure.
// ............................................................................
#define LLN_EMPHASIS        200


// ............................................................................
// LLN_CONTEXTMENU
//     The WC_LVMDISKS control or one of its children requested a popup menu.
//     mp2 is a pointer to a DISKNOTIFY structure.
// ............................................................................
#define LLN_CONTEXTMENU     201


// ............................................................................
// LLN_SETFOCUS
//     The disk list is gaining input focus.  mp2 contains the window handle.
// ............................................................................
#define LLN_SETFOCUS        202


// ............................................................................
// LLN_KILLFOCUS
//     The disk list is losing input focus.  mp2 contains the window handle.
// ............................................................................
#define LLN_KILLFOCUS       203



// ----------------------------------------------------------------------------
// TYPEDEFS

#ifndef LVM_GLBS_H_INCLUDED
  typedef unsigned long CARDINAL32;
  #undef ADDRESS
  typedef void * ADDRESS;
#endif


// Structure used by various WM_CONTROL notification messages
typedef struct _Notify_Disk_Emphasis {
    HWND    hwndDisk,        // HWND of the WC_DISKVIEW control
            hwndPartition;   // HWND of the WC_PARTITIONVIEW control, if any
    ADDRESS disk,            // LVM disk handle
            partition;       // LVM partition handle, if any
    USHORT  usDisk,          // Disk array index number (meaningless if hwndDisk is NULLHANDLE)
            usPartition;     // Partition array index number (meaningless if hwndPartition is NULLHANDLE)
} DISKNOTIFY, *PDISKNOTIFY;


// Control data for the WC_PARTITIONVIEW control (used by WM_CREATE etc.)
typedef struct _PartitionView_CtlData {
    USHORT     cb;                              // size of this structure
    ADDRESS    handle;                          // LVM partition handle
    CARDINAL32 number;                          // LVM partition number on disk
    CARDINAL32 disk;                            // LVM disk number of partition
    BYTE       bType;                           // partition type (LPV_TYPE_*)
    BYTE       bOS;                             // OS flag
    ULONG      ulSize;                          // partition size in MiB
    CHAR       szName[ PARTITION_NAME_SIZE+1 ]; // partition name
    CHAR       szFS[ FILESYSTEM_NAME_SIZE+1 ];  // name of filesystem on partition (if any)
    CHAR       cLetter;                         // associated drive letter
    BOOL       fInUse;                          // partition is in use (by a volume or BM)
    BOOL       fDisable;                        // partition is unselectable
} PVCTLDATA, *PPVCTLDATA;


// Control data for the WC_DISKVIEW control (used by WM_CREATE etc.)
typedef struct _DiskView_CtlData {
    USHORT     cb;                              // size of this structure
    ADDRESS    handle;                          // LVM engine disk handle
    CARDINAL32 number;                          // LVM disk number (from 1)
    ULONG      ulSize;                          // disk size in MiB
    CHAR       szName[ DISK_NAME_SIZE+1 ],      // disk name string
               szSize[ SIZE_TEXT_LIMIT+1 ];     // disk size string
} DVCTLDATA, *PDVCTLDATA;


// Control data for the WC_VOLUMEINFO (used by WM_CREATE etc.)
typedef struct _VolInfo_CtlData {
    USHORT      cb;         // size of this structure
    ULONG       ulCnrID;    // resource ID to use for the partition container
    USHORT      fsStyle;    // style flags
#ifdef VICTLDATA_STRINGS
    PSZ      pszFSL,        // NLS-text for the filesystem label
             pszDeviceL,    // NLS-text for the device type label
             pszSizeL;      // NLS-text for the size label
#endif
} VICTLDATA, *PVICTLDATA;


// Structure for setting the displayed volume information (used by VIM_SETINFO)
typedef struct _VolInfo_Info {
    HPOINTER hIcon;         // handle of volume icon
#ifndef VICTLDATA_STRINGS
    PSZ      pszFSL,        // NLS-text for the filesystem label
             pszDeviceL,    // NLS-text for the device type label
             pszSizeL;      // NLS-text for the size label
#endif
    PSZ      pszLetter;     // drive-letter string
    PSZ      pszName;       // volume name
    PSZ      pszType;       // volume type
    PSZ      pszDevice;     // device type
    PSZ      pszFS;         // volume filesystem
    PSZ      pszSize;       // volume size string
} VIVOLUMEINFO, *PVIVOLUMEINFO;


// ----------------------------------------------------------------------------
// FUNCTION PROTOTYPES

BOOL PVRegisterClass( HAB hab );
BOOL DVRegisterClass( HAB hab );
BOOL DLRegisterClass( HAB hab );
BOOL VIRegisterClass( HAB hab );
LONG GetCurrentDPI( HWND hwnd );
BOOL GetBoldFontPP( HPS hps, PSZ pszFont );
BOOL GetImageFont( HPS hps, PSZ pszFontFace, PFATTRS pfAttrs, LONG lCY );
void SetFontFromPP( HPS hps, HWND hwnd, LONG lDPI );


