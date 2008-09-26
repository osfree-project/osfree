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
           $(p)mmi_install.obj$(e) $(p)file$(e) $(p)globstr$(e) $(p)mciini$(e) &
           $(p)msg$(e) $(p)dll$(e) $(p)crc$(e)
DLL     = 1
DLLOPT  = initinstance terminstance op manyautodata
# defines additional options for C compiler
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
exports = MINSTALL_Init MINSTALL, &
          MINSTALL_InitPackage MINSTALL, &
          MINSTALL_InstallPackage MINSTALL, &
          MINSTALL_GetErrorMsgPtr MINSTALL, &
          MINSTALL_GetErrorMsgCIDCode MINSTALL, &
          MINSTALL_GetErrorMsgCIDCodeDescription MINSTALL, &
          MINSTALL_GetSourcePathPtr MINSTALL, &
          MINSTALL_GetTargetPathPtr MINSTALL, &
          MINSTALL_GetPublicGroupArrayPtr MINSTALL, &
          MINSTALL_GetPublicGroupCustomDataPtr MINSTALL, &
          MINSTALL_SetPublicGroupCustomData MINSTALL, &
          MINSTALL_GetPackageTitlePtr MINSTALL, &
          MINSTALL_SelectGroup MINSTALL, &
          MINSTALL_DeSelectGroup MINSTALL, &
          MINSTALL_SetCARDINFOCardCountForGroup MINSTALL, &
          MINSTALL_GetCARDINFOChoiceForGroup MINSTALL, &
          MINSTALL_SetCARDINFOChoiceForGroup MINSTALL, &
          MINSTALL_CleanUp MINSTALL
ADD_LINKOPT = lib mm_shared

!include $(%ROOT)/mk/appsos2.mk
