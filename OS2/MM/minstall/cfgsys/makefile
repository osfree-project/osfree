#
# A Makefile for CFGSYS
# (c) osFree project,
# author, date
#

PROJ = cfgsys
DESC = CFGSYS
ADD_COPT = -i=$(MYDIR)..$(SEP)include
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)configsys$(e) $(p)dosh$(e) $(p)stringh$(e) &
           $(p)xstring$(e) $(p)nls$(e) $(p)prfh$(e)
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
