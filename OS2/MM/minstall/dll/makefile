#
# A Makefile for MINSTALL
# (c) osFree project,
# author, date
#

PROJ = minstall
DESC = MINSTALL
ADD_COPT = -i=$(MYDIR)..$(SEP)include
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)mmi_main$(e) $(p)mmi_basescr$(e) $(p)mmi_ctrlscr$(e) &
           $(p)mmi_ctrlprc$(e) $(p)mmi_customdll$(e) $(p)mmi_imports$(e) &
           $(p)mmi_inistuff$(e) $(p)mmi_cardinfo$(e) $(p)mmi_helper$(e) &
           $(p)mmi_install.obj$(e) # $(p)file$(e) $(p)globstr$(e) $(p)mciini$(e) &
           $(p)msg$(e) $(p)dll$(e) $(p)crc$(e)
DLL     = 1
DLLOPT  = initinstance terminstance op manyautodata
# defines additional options for C compiler
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
imports = CONFIGSYS_Process CFGSYS, &
          CONFIGSYS_DelayInit CFGSYS, &
          CONFIGSYS_DelayCopyFile CFGSYS, &
          CONFIGSYS_DelayFinalize CFGSYS

exports = MINSTALL_Init, &
          MINSTALL_InitPackage, &
          MINSTALL_InstallPackage, &
          MINSTALL_GetErrorMsgPtr, &
          MINSTALL_GetErrorMsgCIDCode, &
          MINSTALL_GetErrorMsgCIDCodeDescription, &
          MINSTALL_GetSourcePathPtr, &
          MINSTALL_GetTargetPathPtr, &
          MINSTALL_GetPublicGroupArrayPtr, &
          MINSTALL_GetPublicGroupCustomDataPtr, &
          MINSTALL_SetPublicGroupCustomData, &
          MINSTALL_GetPackageTitlePtr, &
          MINSTALL_SelectGroup, &
          MINSTALL_DeSelectGroup, &
          MINSTALL_SetCARDINFOCardCountForGroup, &
          MINSTALL_GetCARDINFOChoiceForGroup, &
          MINSTALL_SetCARDINFOChoiceForGroup, &
          MINSTALL_CleanUp
ADD_LINKOPT = lib mm_shared

!include $(%ROOT)/mk/appsos2.mk
