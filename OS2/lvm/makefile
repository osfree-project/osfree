#
# A Makefile for viocalls.dll
# (c) osFree project,
# author, date
#

PROJ     = lvm
TRGT     = $(PROJ).dll
DESC     = LVM Engine
srcfiles = $(p)dllstart$(e) $(p)engine$(e) $(p)Partition_Manager$(e) $(p)volume$(e) $(p)BootManager$(e) $(p)diskio$(e) $(p)logging$(e) $(p)dlist$(e) $(p)handlemgr$(e) $(p)crc$(e) $(p)Bad_Block_Relocation$(e) $(p)Drive_Linking$(e) $(p)Pass_Thru$(e)
ADD_COPT    = -i=..$(SEP)include -s -ecs
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
UNI2H = 0

EXPORTS = Get_Partition_Handle .1, &
Remove_Boot_Manager .2, &
Get_Boot_Manager_Menu .3, &
Assign_Drive_Letter .4, &
ADD_TO_BOOT_MANAGER16 .5, &
REMOVE_BOOT_MANAGER16 .6, &
Set_Min_Install_Size .7, &
INSTALL_BOOT_MANAGER16 .8, &
Export_Configuration .9, &
ASSIGN_DRIVE_LETTER16 .10, &
GET_PARTITION_HANDLE16 .11, &
SET_MIN_INSTALL_SIZE16 .12, &
EXPORT_CONFIGURATION16 .13, &
Get_Installable_Volume .14, &
Hide_Volume .15, &
Get_Drive_Control_Data .16, &
GET_BOOT_MANAGER_MENU16 .17, &
Read_Sectors .18, &
HIDE_VOLUME16 .19, &
Get_Volume_Information .20, &
Set_OS_Flag .21, &
Get_LVM_View .22, &
Stop_Logging .23, &
Set_Startable .24, &
Start_Logging .25, &
SET_OS_FLAG16 .26, &
GET_INSTALLABLE_VOLUME16 .27, &
READ_SECTORS16 .28, &
GET_DRIVE_CONTROL_DATA16 .29, &
Create_Volume .30, &
Delete_Volume .31, &
GET_LVM_VIEW16 .32, &
Write_Sectors .33, &
STOP_LOGGING16 .34, &
Get_Boot_Manager_Handle .35, &
Expand_Volume .36, &
GET_VOLUME_INFORMATION16 .37, &
GET_BOOT_MANAGER_HANDLE16 .38, &
Get_Partitions .39, &
SET_STARTABLE16 .40, &
Commit_Changes .41, &
Get_Boot_Manager_Options .42, &
START_LOGGING16 .43, &
Remove_From_Boot_Manager .44, &
Set_Boot_Manager_Options .45, &
Set_Free_Space_Threshold .46, &
CREATE_VOLUME16 .47, &
Boot_Manager_Is_Installed .48, &
DELETE_VOLUME16 .49, &
Get_Volume_Control_Data .50, &
WRITE_SECTORS16 .51, &
EXPAND_VOLUME16 .52, &
GET_BOOT_MANAGER_OPTIONS16 .53, &
Get_Partition_Information .54, &
REMOVE_FROM_BOOT_MANAGER16 .55, &
SET_BOOT_MANAGER_OPTIONS16 .56, &
GET_VOLUME_CONTROL_DATA16 .57, &
GET_PARTITIONS16 .58, &
COMMIT_CHANGES16 .59, &
SET_FREE_SPACE_THRESHOLD16 .60, &
Changes_Pending .61, &
CHANGES_PENDING16 .62, &
Set_Installable .63, &
Get_Reboot_Flag .64, &
Open_LVM_Engine .65, &
Set_Reboot_Flag .66, &
Reboot_Required .67, &
BOOT_MANAGER_IS_INSTALLED16 .68, &
Set_Active_Flag .69, &
Rediscover_PRMs .70, &
SET_INSTALLABLE16 .71, &
Close_LVM_Engine .72, &
GET_REBOOT_FLAG16 .73, &
OPEN_LVM_ENGINE16 .74, &
SET_REBOOT_FLAG16 .75, &
REBOOT_REQUIRED16 .76, &
GET_PARTITION_INFORMATION16 .77, &
SET_ACTIVE_FLAG16 .78, &
Create_Partition .79, &
Get_Reserved_Drive_Letters .80, &
Delete_Partition .81, &
REDISCOVER_PRMS16 .82, &
Get_Install_Flags .83, &
Get_Drive_Status .84, &
Set_Install_Flags .85, &
Get_Valid_Options .86, &
CLOSE_LVM_ENGINE16 .87, &
CREATE_PARTITION16 .88, &
DELETE_PARTITION16 .89, &
GET_DRIVE_STATUS16 .90, &
Refresh_LVM_Engine .91, &
GET_RESERVED_DRIVE_LETTERS16 .92, &
Get_Available_Drive_Letters .93, &
Free_Engine_Memory .94, &
GET_AVAILABLE_DRIVE_LETTERS16 .95, &
New_MBR .96, &
GET_INSTALL_FLAGS16 .97, &
Set_Name .98, &
SET_INSTALL_FLAGS16 .99, &
NEW_MBR16 .100, &
GET_VALID_OPTIONS16 .101, &
REFRESH_LVM_ENGINE16 .102, &
FREE_ENGINE_MEMORY16 .103, &
SET_NAME16 .104, &
Install_Boot_Manager .105, &
Add_To_Boot_Manager .106

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
