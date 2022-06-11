#
# A Makefile for viocalls.dll
# (c) osFree project,
# author, date
#

PROJ     = lvm
TRGT     = $(PROJ).dll
DESC     = LVM Engine
srcfiles = $(p)dllstart$(e) $(p)engine$(e) $(p)Partition_Manager$(e) $(p)Volume_Manager$(e) $(p)BootManager$(e) $(p)DiskIO$(e) $(p)logging$(e) $(p)Dlist$(e) $(p)Handle_Manager$(e) $(p)CRC$(e) $(p)Bad_Block_Relocation$(e) $(p)Drive_Linking$(e) $(p)Pass_Thru$(e)
ADD_COPT    = -i=..$(SEP)include -s -ecs
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
UNI2H = 0

EXPORTS = Open_LVM_Engine, &
Open_LVM_Engine2, &
Commit_Changes, &
Set_Java_Call_Back, &
Close_LVM_Engine, &
Issue_Feature_Command, &
Parse_Feature_Parameters, &
Refresh_LVM_Engine, &
Get_Drive_Control_Data, &
Get_Drive_Status, &
Get_Partitions, &
Get_Partition_Handle.1, &
Get_Partition_Information, &
Create_Partition, &
Delete_Partition, &
Set_Active_Flag, &
Set_OS_Flag, &
Get_Volume_Control_Data, &
Get_Volume_Information, &
Create_Volume, &
Create_Volume2, &
Delete_Volume, &
Hide_Volume, &
Expand_Volume, &
Assign_Drive_Letter, &
Set_Installable, &
Get_Installable_Volume, &
Convert_Volumes_To_V1, &
Set_Name, &
Set_Startable, &
Get_Valid_Options, &
Get_Child_Handles, &
Get_Parent_Handle, &
Get_Features, &
Boot_Manager_Is_Installed, &
Get_Boot_Manager_Handle, &
Add_To_Boot_Manager, &
Remove_From_Boot_Manager, &
Get_Boot_Manager_Menu, &
Install_Boot_Manager, &
Remove_Boot_Manager, &
Set_Boot_Manager_Options, &
Get_Boot_Manager_Options, &
Allocate_Engine_Memory, &
Free_Engine_Memory, &
New_MBR, &
Get_Available_Drive_Letters, &
Get_Reserved_Drive_Letters, &
Reboot_Required, &
Changes_Pending, &
Set_Reboot_Flag, &
Get_Reboot_Flag, &
Set_Install_Flags, &
Get_Install_Flags, &
Set_Min_Install_Size, &
Set_Free_Space_Threshold, &
Read_Sectors, &
Write_Sectors, &
Rediscover_PRMs, &
Get_LVM_View, &
Start_Logging, &
Stop_Logging, &
Export_Configuration, &
OPEN_LVM_ENGINE16, &
OPEN_LVM_ENGINE216, &
COMMIT_CHANGES16, &
CLOSE_LVM_ENGINE16, &
REFRESH_LVM_ENGINE16, &
GET_DRIVE_CONTROL_DATA16, &
GET_DRIVE_STATUS16, &
GET_PARTITIONS16, &
GET_PARTITION_HANDLE16, &
GET_PARTITION_INFORMATION16, &
CREATE_PARTITION16, &
DELETE_PARTITION16, &
SET_ACTIVE_FLAG16, &
SET_OS_FLAG16, &
GET_VOLUME_CONTROL_DATA16, &
GET_VOLUME_INFORMATION16, &
CREATE_VOLUME16, &
CREATE_VOLUME216, &
DELETE_VOLUME16, &
HIDE_VOLUME16, &
EXPAND_VOLUME16, &
ASSIGN_DRIVE_LETTER16, &
SET_INSTALLABLE16, &
GET_INSTALLABLE_VOLUME16, &
SET_NAME16, &
SET_STARTABLE16, &
GET_VALID_OPTIONS16, &
BOOT_MANAGER_IS_INSTALLED16, &
GET_BOOT_MANAGER_HANDLE16, &
ADD_TO_BOOT_MANAGER16, &
REMOVE_FROM_BOOT_MANAGER16, &
GET_BOOT_MANAGER_MENU16, &
INSTALL_BOOT_MANAGER16, &
REMOVE_BOOT_MANAGER16, &
SET_BOOT_MANAGER_OPTIONS16, &
GET_BOOT_MANAGER_OPTIONS16, &
FREE_ENGINE_MEMORY16, &
NEW_MBR16, &
GET_AVAILABLE_DRIVE_LETTERS16, &
GET_RESERVED_DRIVE_LETTERS16, &
REBOOT_REQUIRED16, &
CHANGES_PENDING16, &
SET_REBOOT_FLAG16, &
GET_REBOOT_FLAG16, &
SET_INSTALL_FLAGS16, &
GET_INSTALL_FLAGS16, &
SET_MIN_INSTALL_SIZE16, &
SET_FREE_SPACE_THRESHOLD16, &
READ_SECTORS16, &
WRITE_SECTORS16, &
REDISCOVER_PRMS16, &
GET_LVM_VIEW16, &
START_LOGGING16, &
STOP_LOGGING16, &
EXPORT_CONFIGURATION16


#2	Remove_Boot_Manager
#3	Get_Boot_Manager_Menu	4	Assign_Drive_Letter
#5	ADD_TO_BOOT_MANAGER16	6	REMOVE_BOOT_MANAGER16
#7	Set_Min_Install_Size	8	INSTALL_BOOT_MANAGER16
#9	Export_Configuration	10	ASSIGN_DRIVE_LETTER16
#11	GET_PARTITION_HANDLE16	12	SET_MIN_INSTALL_SIZE16
#13	EXPORT_CONFIGURATION16	14	Get_Installable_Volume
#15	Hide_Volume	16	Get_Drive_Control_Data
#17	GET_BOOT_MANAGER_MENU16	18	Read_Sectors
#19	HIDE_VOLUME16	20	Get_Volume_Information
#21	Set_OS_Flag	22	Get_LVM_View
#23	Stop_Logging	24	Set_Startable
#25	Start_Logging	26	SET_OS_FLAG16
#27	GET_INSTALLABLE_VOLUME16	28	READ_SECTORS16
#29	GET_DRIVE_CONTROL_DATA16	30	Create_Volume
#31	Delete_Volume	32	GET_LVM_VIEW16
#33	Write_Sectors	34	STOP_LOGGING16
#35	Get_Boot_Manager_Handle	36	Expand_Volume
#37	GET_VOLUME_INFORMATION16	38	GET_BOOT_MANAGER_HANDLE16
#39	Get_Partitions	40	SET_STARTABLE16
#41	Commit_Changes	42	Get_Boot_Manager_Options
#43	START_LOGGING16	44	Remove_From_Boot_Manager
#45	Set_Boot_Manager_Options	46	Set_Free_Space_Threshold
#47	CREATE_VOLUME16	48	Boot_Manager_Is_Installed
#49	DELETE_VOLUME16	50	Get_Volume_Control_Data
#51	WRITE_SECTORS16	52	EXPAND_VOLUME16
#53	GET_BOOT_MANAGER_OPTIONS16	54	Get_Partition_Information
#55	REMOVE_FROM_BOOT_MANAGER16	56	SET_BOOT_MANAGER_OPTIONS16
#57	GET_VOLUME_CONTROL_DATA16	58	GET_PARTITIONS16
#59	COMMIT_CHANGES16	60	SET_FREE_SPACE_THRESHOLD16
#61	Changes_Pending	62	CHANGES_PENDING16
#63	Set_Installable	64	Get_Reboot_Flag
#65	Open_LVM_Engine	66	Set_Reboot_Flag
#67	Reboot_Required	68	BOOT_MANAGER_IS_INSTALLED16
#69	Set_Active_Flag	70	Rediscover_PRMs
#71	SET_INSTALLABLE16	72	Close_LVM_Engine
#73	GET_REBOOT_FLAG16	74	OPEN_LVM_ENGINE16
#75	SET_REBOOT_FLAG16	76	REBOOT_REQUIRED16
#77	GET_PARTITION_INFORMATION16	78	SET_ACTIVE_FLAG16
#79	Create_Partition	80	Get_Reserved_Drive_Letters
#81	Delete_Partition	82	REDISCOVER_PRMS16
#83	Get_Install_Flags	84	Get_Drive_Status
#85	Set_Install_Flags	86	Get_Valid_Options
#87	CLOSE_LVM_ENGINE16	88	CREATE_PARTITION16
#89	DELETE_PARTITION16	90	GET_DRIVE_STATUS16
#91	Refresh_LVM_Engine	92	GET_RESERVED_DRIVE_LETTERS16
#93	Get_Available_Drive_Letters	94	Free_Engine_Memory
#95	GET_AVAILABLE_DRIVE_LETTERS16	96	New_MBR
#97	GET_INSTALL_FLAGS16	98	Set_Name
#99	SET_INSTALL_FLAGS16	100	NEW_MBR16
#101	GET_VALID_OPTIONS16	102	REFRESH_LVM_ENGINE16
#103	FREE_ENGINE_MEMORY16	104	SET_NAME16
#105	Install_Boot_Manager	106	Add_To_Boot_Manager

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk
