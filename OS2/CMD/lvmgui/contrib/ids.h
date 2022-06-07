/****************************************************************************
 ** COMMON IDS                                                             **
 *****************************************************************************
 *                                                                           *
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify it  *
 *  under the terms of the GNU General Public License as published by the    *
 *  Free Software Foundation; either version 2 of the License, or (at your   *
 *  option) any later version.                                               *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  General Public License for more details.                                 *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License along  *
 *  with this program; if not, write to the Free Software Foundation, Inc.,  *
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                  *
 *                                                                           *
 *****************************************************************************/

// Main program ID
#define ID_MAINPROGRAM              1

// IDs for common commands
#define DID_PREVIOUS                3
#define DID_HELP                    9

#define ID_LVM_SAVE                 10
#define ID_LVM_REFRESH              11
#define ID_LVM_NEWMBR               12
#define ID_LVM_DISK                 13
#define ID_LVM_EXIT                 18
#define ID_VERIFYQUIT               19

#define ID_AIRBOOT_INSTALL          20
#define ID_AIRBOOT_REMOVE           21
#define ID_BM_INSTALL               23
#define ID_BM_REMOVE                24
#define ID_BM_OPTIONS               25

#define ID_VOLUME_CREATE            30
#define ID_VOLUME_DELETE            31
#define ID_VOLUME_RENAME            32
#define ID_VOLUME_LETTER            33
#define ID_VOLUME_BOOTABLE          34
#define ID_VOLUME_STARTABLE         35

#define ID_PARTITION_CREATE         40
#define ID_PARTITION_DELETE         41
#define ID_PARTITION_RENAME         42
#define ID_PARTITION_CONVERT        43
#define ID_PARTITION_ADD            44
#define ID_PARTITION_BOOTABLE       45
#define ID_PARTITION_STARTABLE      46
#define ID_PARTITION_ACTIVE         47

#define ID_PREFS                    50
#define ID_FONTS                    51

#define ID_HELP_GENERAL             60
#define ID_HELP_TERMS               61
#define ID_HELP_KEYS                62
#define ID_HELP_ABOUT               69

#define ID_FOCUS_FORWARD            70
#define ID_FOCUS_BACK               71


/****************************************************************************
 ** MENU IDS                                                               **
 ****************************************************************************/

#define IDM_LVM                         80
#define IDM_LVM_BOOTMGR                 81
#define IDM_VOLUME                      82
#define IDM_PARTITION                   83
#define IDM_OPTIONS                     84
#define IDM_HELP                        85

#define IDM_CONTEXT_VOLUME              86
#define IDM_CONTEXT_PARTITION           87
#define IDM_CONTEXT_DISK                88

#define IDM_BM_SEPARATOR                89

/****************************************************************************
 ** GRAPHIC RESOURCE IDS                                                   **
 ****************************************************************************/

#define IDP_HDD                         90
#define IDP_PRM                         91
#define IDP_PRM_MISSING                 92
#define IDP_DISK_BAD                    93
#define IDP_MEMDISK                     94
#define IDP_VOL_BASIC                   95
#define IDP_VOL_ADVANCED                96
#define IDP_VOL_UNKNOWN                 97
#define IDP_VOL_CDROM                   98
#define IDP_VOL_NETWORK                 99


/****************************************************************************
 ** DIALOG & WINDOW RESOURCE IDS                                           **
 ****************************************************************************/

/* Controls on the main program window
 */

#define IDD_SPLIT_WINDOW                100

#define IDD_LOGICAL                     101
#define IDD_VOLUMES                     102
#define IDD_VOL_INFO                    103
#define IDD_VOL_PARTITIONS              104
#define IDD_DISKLIST                    110
#define IDD_TOOLTIP                     111
#define IDD_STATUS_SELECTED             121
#define IDD_STATUS_SIZE                 122
#define IDD_STATUS_TYPE                 123
#define IDD_STATUS_FLAGS                124
#define IDD_STATUS_MODIFIED             125

/* Common IDs for various static controls used on multiple secondary dialogs
 */
#define IDD_DIALOG_INSET                190
#define IDD_DIALOG_INSET2               191
#define IDD_DIALOG_DIVIDER              192

/* The Rename Disk dialog
 */
#define IDD_DISK_NAME                   200
#define IDD_DISK_SERIAL_TEXT            201
#define IDD_DISK_SERIAL_FIELD           202
#define IDD_DISK_NAME_TEXT              203
#define IDD_DISK_NAME_FIELD             204

/* The Boot Manager Options dialog
 */
#define IDD_BOOTMGR_OPTIONS             300
#define IDD_BOOTMGR_DEFAULT_TEXT        301
#define IDD_BOOTMGR_DEFAULT_LIST        302
#define IDD_BOOTMGR_TIMEOUT_ENABLE      303
#define IDD_BOOTMGR_TIMEOUT_TEXT        304
#define IDD_BOOTMGR_TIMEOUT_VALUE       305
#define IDD_BOOTMGR_MODE_BRIEF          306
#define IDD_BOOTMGR_MODE_VERBOSE        307

/* The Create New Volume (1) dialog.  This dialog template will also be used
 * for the Convert to Volume function.
 */
#define IDD_VOLUME_CREATE_1             400
#define IDD_VOLUME_CREATE_INTRO         401
#define IDD_VOLUME_CREATE_STANDARD      402
#define IDD_VOLUME_CREATE_ADVANCED      403
#define IDD_VOLUME_CREATE_BOOTABLE      404

/* The Create New Volume (2) dialog.  This dialog template will also be used
 * for the Expand Volume function.
 */
#define IDD_VOLUME_CREATE_2             500
#define IDD_VOLUME_CREATE_SELECT        501
#define IDD_VOLUME_CREATE_LIST          502
#define IDD_VOLUME_CREATE_STATUS        503
#define IDD_VOLUME_CREATE_CONTENTS      504
#define IDD_VOLUME_CREATE_ADD           505
#define IDD_VOLUME_CREATE_REMOVE        506

/* The Volume Drive Letter dialog (some of these are also used on the Create
 * Volume (1) dialog)
 */
#define IDD_VOLUME_LETTER               600
#define IDD_VOLUME_LETTER_INTRO         601
#define IDD_VOLUME_LETTER_TEXT          602
#define IDD_VOLUME_LETTER_LIST          603
#define IDD_VOLUME_LETTER_NOTE          604

/* The Volume Name dialog (some of these are also used on the Create Volume (1)
 * dialog)
 */
#define IDD_VOLUME_NAME                 700
#define IDD_VOLUME_NAME_INTRO           701
#define IDD_VOLUME_NAME_TEXT            702
#define IDD_VOLUME_NAME_FIELD           703

/* The Create Partition dialog
 */
#define IDD_PARTITION_CREATE            800
#define IDD_PARTITION_CREATE_NAME_INTRO 801
#define IDD_PARTITION_CREATE_NAME_TEXT  802
#define IDD_PARTITION_CREATE_NAME_FIELD 803
#define IDD_PARTITION_CREATE_SIZE_INTRO 804
#define IDD_PARTITION_CREATE_SIZE_TEXT  805
#define IDD_PARTITION_CREATE_SIZE_SPIN  806
#define IDD_PARTITION_CREATE_SIZE_MB    807
#define IDD_PARTITION_CREATE_PRIMARY    808
#define IDD_PARTITION_CREATE_FROM_END   809
#define IDD_PARTITION_CREATE_INTRO      810

/* The Add Partition to Volume dialog
 */
#define IDD_PARTITION_ADD               1000
#define IDD_PARTITION_ADD_TEXT          1001
#define IDD_PARTITION_ADD_VOLUMES       1002

/* The Preferences dialog
 */
#define IDD_PREFERENCES                 1100
#define IDD_PREFS_TERMS_IBM             1101
#define IDD_PREFS_TERMS_IEC             1102
#define IDD_PREFS_BOOT_WARNING          1103
#define IDD_PREFS_STYLE_TEXT            1104
#define IDD_PREFS_STYLE_PM              1105
#define IDD_PREFS_STYLE_WEB             1106
#define IDD_PREFS_ENABLE_BM             1107
#define IDD_PREFS_ENABLE_AB             1108
#define IDD_PREFS_NO_EMPTY_DISKS        1109
#define IDD_PREFS_NO_ALIEN_VOLUMES      1110
#define IDD_PREFS_UNIFORM               1111
#define IDD_PREFS_AUTOSELECT            1112

/* The Fonts dialog
 */
#define IDD_FONTS                       1200
#define IDD_FONTS_CONTAINER_TEXT        1201
#define IDD_FONTS_CONTAINER_PREVIEW     1202
#define IDD_FONTS_CONTAINER_BTN         1203
#define IDD_FONTS_DETAILS_TEXT          1204
#define IDD_FONTS_DETAILS_PREVIEW       1205
#define IDD_FONTS_DETAILS_BTN           1206
#define IDD_FONTS_DISKS_TEXT            1207
#define IDD_FONTS_DISKS_PREVIEW         1208
#define IDD_FONTS_DISKS_BTN             1209
#define IDD_FONTS_STATUS_TEXT           1210
#define IDD_FONTS_STATUS_PREVIEW        1211
#define IDD_FONTS_STATUS_BTN            1212
#define IDD_FONTS_DIALOG_TEXT           1213
#define IDD_FONTS_DIALOG_PREVIEW        1214
#define IDD_FONTS_DIALOG_BTN            1215
#define IDD_FONTS_DIALOG_CLEAR          1216

/* The Product Information dialog
 */
#define IDD_ABOUT                       1900
#define IDD_ABOUT_ICON                  1901
#define IDD_ABOUT_NAME                  1902
#define IDD_ABOUT_VERSION               1903
#define IDD_ABOUT_BUILD                 1904
#define IDD_ABOUT_AUTHOR                1905
#define IDD_ABOUT_LICENSE               1906


/* LVM error message box ID.
 */
// This number plus API error code derives the actual help string ID
#define IDD_ENGINE_ERROR                2000
// Other values
#define IDD_ENGINE_ERROR_INCOMPATIBLE   2070


/****************************************************************************
 ** HELPTABLE IDS                                                          **
 ****************************************************************************/

#define IDH_MAIN                        10000
#define IDH_DISK_NAME                   10010
#define IDH_BOOTMGR_OPTIONS             10020
#define IDH_VOLUME_CREATE_1             10030
#define IDH_VOLUME_CREATE_2             10040
#define IDH_VOLUME_LETTER               10050
#define IDH_VOLUME_NAME                 10060
#define IDH_PARTITION_CREATE            10070
#define IDH_PARTITION_ADD               10080
#define IDH_PREFERENCES                 10090
#define IDH_FONTS                       10100
#define IDH_ENGINE_ERROR                10200


/****************************************************************************
 ** STRING RESOURCE IDS                                                    **
 ****************************************************************************
 * As per the PM docs, in order to optimise efficiency we will try as much  *
 * as possible to bundle string resources together in groups of 16 (with    *
 * each group starting at some factor of 16).  We still have a few gaps     *
 * here and there, but hopefully most of those can be filled in eventually. *
 ****************************************************************************/

/* Help-manager related strings
 */
#define IDS_HELP_TITLE                  11008
#define IDS_HELP_LOAD_ERROR             11009
#define IDS_HELP_DISP_ERROR             11010
#define IDS_HELP_ERROR_TITLE            11011

/* General program popup messages
 */
#define IDS_PROGRAM_TITLE               11012
#define IDS_SAVE_TITLE                  11013
#define IDS_SAVE_QUIT                   11014
#define IDS_SAVE_CONFIRM                11015
#define IDS_REBOOT_TITLE                11016
#define IDS_REBOOT_NOTIFY               11017
#define IDS_NOBOOT_TITLE                11018
#define IDS_NOBOOT_WARNING              11019
#define IDS_SAVE_WARNING                11020
#define IDS_PROCEED_CONFIRM             11021
#define IDS_CONFIRM_TITLE               11022
#define IDS_SUCCESS_TITLE               11023

/* User interface/control text strings
 */
#define IDS_STATUS_PARTITION            11024
#define IDS_STATUS_VOLUME_HIDDEN        11025
#define IDS_STATUS_VOLUME_LETTER        11026
#define IDS_STATUS_TYPE                 11027
#define IDS_STATUS_FORMAT               11028
#define IDS_STATUS_AVAILABLE            11029
#define IDS_STATUS_INUSE                11030
#define IDS_STATUS_FREESPACE            11031
#define IDS_STATUS_STARTABLE            11032
#define IDS_STATUS_BOOTABLE             11033
#define IDS_STATUS_INSTALLABLE          11034
#define IDS_STATUS_SPANNED              11035
#define IDS_STATUS_MODIFIED             11036
#define IDS_STATUS_PARTITION_SHORT      11037
#define IDS_STATUS_FREESPACE_SHORT      11038
#define IDS_STATUS_UNUSABLE             11039

#define IDS_DEVICE_HDD                  11040
#define IDS_DEVICE_PRM                  11041
#define IDS_DEVICE_CD                   11042
#define IDS_DEVICE_LAN                  11043
#define IDS_DEVICE_UNKNOWN              11044
#define IDS_DEVICE_MEMDISK              11045
#define IDS_TERMS_PRIMARY               11046
#define IDS_TERMS_LOGICAL               11047
#define IDS_TERMS_STANDARD              11048
#define IDS_TERMS_COMPATIBILITY         11049
#define IDS_TERMS_ADVANCED              11050
#define IDS_TERMS_LVM                   11051
#define IDS_OS_OSTYPE                   11052
#define IDS_LETTER_CHANGED              11053
#define IDS_VOLUME_TYPE_LONG            11054
#define IDS_TERMS_FREE                  11055

// various labels and column text
#define IDS_LABELS_FILESYSTEM           11056
#define IDS_LABELS_SIZE                 11057
#define IDS_LABELS_VOLUMES              11058
#define IDS_LABELS_DISKS                11059
#define IDS_LABELS_PARTITIONS           11060
#define IDS_LABELS_DEVICE               11061
#define IDS_LABELS_DEFAULT_FONT         11062
#define IDS_FIELD_VOLUME_NAME           11063
#define IDS_FIELD_SIZE                  11064
#define IDS_FIELD_FILESYSTEM            11065
#define IDS_FIELD_VOLUME_TYPE           11066
#define IDS_FIELD_FLAGS                 11067
#define IDS_FIELD_PARTITION_NAME        11068
#define IDS_FIELD_DISK                  11069
#define IDS_TOOLTIP_PARTITION           11070

// dynamically added menu items
#define IDS_MENU_AIRBOOT_INSTALL        11072
#define IDS_MENU_AIRBOOT_REMOVE         11073
#define IDS_MENU_BOOTMGR_INSTALL        11074
#define IDS_MENU_BOOTMGR_REMOVE         11075
#define IDS_MENU_BOOTMGR_OPTIONS        11076
#define IDS_MENU_BOOTABLE               11077
#define IDS_MENU_NO_BOOTMGR             11078

// other miscellenous UI strings
#define IDS_UITEXT_OK                   11088
#define IDS_UITEXT_CREATE               11089
#define IDS_UITEXT_NOTALLOWED           11090

// strings for Boot Manager
#define IDS_BOOTMGR_INSTALL             11168
#define IDS_BOOTMGR_INSTALL_TITLE       11169
#define IDS_BOOTMGR_WRONG_DISK          11170
#define IDS_BOOTMGR_WRONG_TITLE         11171
#define IDS_BOOTMGR_EMPTY               11172
#define IDS_BOOTMGR_EMPTY_TITLE         11173
#define IDS_BOOTMGR_LAST_BOOTED         11174
#define IDS_BOOTMGR_DELETE              11175
#define IDS_BOOTMGR_DELETE_TITLE        11176

// strings for Air-Boot
#define IDS_AIRBOOT_INSTALL_TITLE       11177
#define IDS_AIRBOOT_INSTALL_1           11178
#define IDS_AIRBOOT_INSTALL_2           11179
#define IDS_AIRBOOT_DELETE              11180
#define IDS_AIRBOOT_FAILED_TITLE        11181
#define IDS_AIRBOOT_FAILED              11182
#define IDS_AIRBOOT_INSTALLED           11183

// other prompt strings
#define IDS_NEWMBR_TITLE                11184
#define IDS_NEWMBR_CONFIRM              11185
#define IDS_NEWMBR_WARN_1               11186
#define IDS_NEWMBR_WARN_2               11187
#define IDS_NEWMBR_OK                   11188

// Strings for secondary dialogs
//

#define IDS_VOLUME_DEFAULT_NAME         11200
#define IDS_VOLUME_NEW_STANDARD         11201
#define IDS_VOLUME_NEW_COMPATIBLE       11202
#define IDS_VOLUME_NEW_ADVANCED         11203
#define IDS_VOLUME_NEW_LVM              11204
#define IDS_VOLUME_NEW_BOOTABLE         11205
#define IDS_VOLUME_NEW_STARTABLE        11206
#define IDS_VOLUME_NEW_SELECT_ONE       11207
#define IDS_VOLUME_NEW_SELECT_SOME      11208
#define IDS_LETTER_NONE                 11209
#define IDS_LETTER_AUTO                 11210
#define IDS_LETTER_INUSE                11211
#define IDS_LETTER_CURRENT              11212
#define IDS_PARTITION_NEW_TITLE         11213
#define IDS_PARTITION_NOT_SELECTED      11214
#define IDS_PARTITION_NOT_FREESPACE     11215

#define IDS_VOLUME_NAME_TITLE           11216
#define IDS_PARTITION_NAME_TITLE        11217
#define IDS_VOLUME_NAME_PROMPT          11218
#define IDS_PARTITION_NAME_PROMPT       11219
#define IDS_VOLUME_DELETE_TITLE         11220
#define IDS_VOLUME_DELETE_CONFIRM       11221
#define IDS_PARTITION_DELETE_TITLE      11222
#define IDS_PARTITION_DELETE_CONFIRM    11223
#define IDS_PARTITION_VOLUME            11224
#define IDS_PARTITION_DELETE_VOLUME     11225

#define IDS_VOLUME_BOOTABLE_TITLE       11232
#define IDS_VOLUME_BOOTABLE_CONFIRM     11233
#define IDS_VOLUME_STARTABLE_TITLE      11234
#define IDS_VOLUME_STARTABLE_CONFIRM    11235
#define IDS_VOLUME_STARTABLE_AIRBOOT    11236
#define IDS_VOLUME_STARTABLE_BOOTMGR    11237
#define IDS_PARTITION_BOOTABLE_TITLE    11238
#define IDS_PARTITION_BOOTABLE_CONFIRM  11239
#define IDS_PARTITION_ACTIVE_TITLE      11240
#define IDS_PARTITION_ACTIVE_CONFIRM    11241
#define IDS_PARTITION_CONVERT_TITLE     11242
#define IDS_PARTITION_NO_LVM_VOLUMES    11243
#define IDS_VOLUME_NOT_AVAILABLE        11244

#define IDS_PRODUCT_VERSION             11904
#define IDS_PRODUCT_REVISION            11905
#define IDS_PRODUCT_COPYRIGHT           11906
#define IDS_GPL_OVERVIEW_1              11907
#define IDS_GPL_OVERVIEW_2              11908
#define IDS_GPL_OVERVIEW_3              11909
#define IDS_GPL_OVERVIEW_4              11910
#define IDS_GPL_OVERVIEW_5              11911
#define IDS_GPL_OVERVIEW_6              11912

/* LVM engine error descriptions
 */

// This number plus API error code derives the actual help string ID
#define IDS_ERROR_LVM               12000

// Special values that don't follow the above rule
#define IDS_ERROR_LVM_INCOMPATIBLE  12070


/* General program error strings
 */
#define IDS_ERROR_GENERIC           13000
#define IDS_ERROR_PROGRAM           13001
#define IDS_ERROR_ENGINE            13002
#define IDS_ERROR_INTERNAL          13003
#define IDS_ERROR_REGISTER          13004
#define IDS_ERROR_INITDLG           13005
#define IDS_ERROR_NODISKS           13006
#define IDS_ERROR_CORRUPT           13007
#define IDS_ERROR_IO                13008
#define IDS_ERROR_GEOMETRY          13009
#define IDS_ERROR_INSUFFICIENT      13010
#define IDS_ERROR_STARTAPP          13011
#define IDS_ERROR_SELECTION         13012
#define IDS_ERROR_UNAVAILABLE       13013
#define IDS_ERROR_OTHER             13099

