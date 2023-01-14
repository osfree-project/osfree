#
# A Makefile for CFGSYS
# (c) osFree project,
# author, date
#

PROJ = cfgsys
DESC = CFGSYS
ADD_COPT = -j -i=$(MYDIR)..$(SEP)include
#defines object file names in format $(p)objname$(e)
srcfiles = $(p)configsys$(e) $(p)dosh$(e) $(p)stringh$(e) &
           $(p)xstring$(e) $(p)nls$(e) $(p)prfh$(e)
#UNI2H   = 1
DLL     = 1
DLLOPT  = initinstance terminstance op manyautodata
# defines additional options for C compiler
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
exports = &
#          CONFIGSYS_Process.1, &
#          CONFIGSYS_DelayInit.10, &
#          CONFIGSYS_DelayFinalize.11, &
#          CONFIGSYS_DelayCopyFile.12, &
#          CONFIGSYS_DelayRemoveFile.13, &
#          CONFIGSYS_DelayCreatePath.14, &
#          CONFIGSYS_DelayRemovePath.15, &
#          ConfigSysLoadFuncs.98, &
#          ConfigSysDropFuncs.99, &
#          ConfigSysDelayInit.110, &
#          ConfigSysProcess.101, &
#          ConfigSysDelayFinalize.111, &
#          ConfigSysDelayCopyFile.112, &
          ConfigSysDelayRemoveFile.113, &
          ConfigSysDelayCreatePath.114, &
          ConfigSysDelayRemovePath.115, &
          ConfigSysRegEnumKey.120, &
          ConfigSysRegCreateKey.122, &
          ConfigSysRegDeleteKey.123, &
          ConfigSysRegEnumValue.124, &
          ConfigSysRegQueryValue.125, &
          ConfigSysRegSetValue.126, &
          ConfigSysRegDeleteValue.127

ADD_LINKOPT = lib mm_shared

!include $(%ROOT)tools/mk/appsos2_cmd.mk
