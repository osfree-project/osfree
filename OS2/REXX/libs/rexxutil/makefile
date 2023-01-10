
DBG = 1
#DBG = 0

!ifeq DBG 1
OPT         = -d3 -of -od
!else
OPT         = -d0 -oteanx
!endif

PROJ        = rexxutil
TRGT        = $(PROJ).dll
DESC        = REXX Utilities API
UNI2H       = 1
# NOLIBS      = 0
C = s
ADD_COPT    = -i=$(MYDIR) -bm -wx -s -mf $(OPT) -fp6 -6s # -6r
# ADD_COPT    = -i=$(MYDIR) -i=..$(SEP)..$(SEP)..$(SEP)CMD$(SEP)include -bm -wx -s -mf $(OPT) -fp6 -6s
DLL         = 1
DLLOPT      = initinstance terminstance
OPTIONS     = manyautodata
STACKSIZE   = 0x20000
C = s

srcfiles    = $(p)rexxfuncs$(e) $(p)sysfile$(e)     $(p)sysmiscfile$(e) $(p)systextscreen$(e) $(p)sysversion$(e)  &
              $(p)sysdrive$(e)  $(p)syseautil$(e)   $(p)sysobjects$(e)  $(p)sysprocess$(e)    $(p)syslang$(e)     &
              $(p)sysmacro$(e)  $(p)sysstem$(e)     $(p)sysmisc$(e)     $(p)sysmutexsem$(e)   $(p)syseventsem$(e) &
              $(p)sysmath$(e)   $(p)helperfuncs$(e) $(p)rxmessage$(e)
EXPORTS = &
   SYSCLS.1                       = SysCls, &
   SYSCURPOS.2                    = SysCurPos, &
   SYSCURSTATE.3                  = SysCurState, &
   SYSDRIVEINFO.4                 = SysDriveInfo, &
   SYSDRIVEMAP.5                  = SysDriveMap, &
   SYSDROPFUNCS.6                 = SysDropFuncs, &
   SYSFILEDELETE.7                = SysFileDelete, &
   SYSFILESEARCH.8                = SysFileSearch, &
   SYSFILETREE.9                  = SysFileTree, &
   SYSGETKEY.10                   = SysGetKey, &
   SYSGETMESSAGE.11               = SysGetMessage, &
   SYSINI.12                      = SysIni, &
   SYSLOADFUNCS.13                = SysLoadFuncs, &
   SYSMKDIR.14                    = SysMkDir, &
   SYSOS2VER.15                   = SysOS2Ver, &
   SYSRMDIR.16                    = SysRmDir, &
   SYSSEARCHPATH.17               = SysSearchPath, &
   SYSSLEEP.18                    = SysSleep, &
   SYSTEMPFILENAME.19             = SysTempFileName, &
   SYSTEXTSCREENREAD.20           = SysTextScreenRead, &
   SYSTEXTSCREENSIZE.21           = SysTextScreenSize, &
   SYSGETEA.22                    = SysGetEA, &
   SYSPUTEA.23                    = SysPutEA, &
   SYSWAITNAMEDPIPE.24            = SysWaitNamedPipe, &
   SYSSETICON.25                  = SysSetIcon, &
   SYSREGISTEROBJECTCLASS.26      = SysRegisterObjectClass, &
   SYSDEREGISTEROBJECTCLASS.27    = SysDeregisterObjectClass, &
   SYSCREATEOBJECT.28             = SysCreateObject, &
   SYSQUERYCLASSLIST.29           = SysQueryClassList, &
   SYSDESTROYOBJECT.30            = SysDestroyObject, &
   SYSSETOBJECTDATA.31            = SysSetObjectData, &
   SYSSAVEOBJECT.32               = SysSaveObject, &
   SYSOPENOBJECT.33               = SysOpenObject, &
   SYSMOVEOBJECT.34               = SysMoveObject, &
   SYSCOPYOBJECT.35               = SysCopyObject, &
   SYSCREATESHADOW.36             = SysCreateShadow, &
   SYSBOOTDRIVE.37                = SysBootDrive, &
   SYSQUERYEALIST.38              = SysQueryEAList, &
   SYSWILDCARD.39                 = SysWildCard, &
   SYSFILESYSTEMTYPE.40           = SysFileSystemType, &
   SYSADDFILEHANDLE.41            = SysAddFileHandle, &
   SYSSETFILEHANDLE.42            = SysSetFileHandle, &
   SYSCREATEMUTEXSEM.43           = SysCreateMutexSem, &
   SYSOPENMUTEXSEM.44             = SysOpenMutexSem, &
   SYSCLOSEMUTEXSEM.45            = SysCloseMutexSem, &
   SYSREQUESTMUTEXSEM.46          = SysRequestMutexSem, &
   SYSRELEASEMUTEXSEM.47          = SysReleaseMutexSem, &
   SYSCREATEEVENTSEM.48           = SysCreateEventSem, &
   SYSOPENEVENTSEM.49             = SysOpenEventSem, &
   SYSCLOSEEVENTSEM.50            = SysCloseEventSem, &
   SYSRESETEVENTSEM.51            = SysResetEventSem, &
   SYSPOSTEVENTSEM.52             = SysPostEventSem, &
   SYSWAITEVENTSEM.53             = SysWaitEventSem, &
   SYSSETPRIORITY.54              = SysSetPriority, &
   SYSPROCESSTYPE.55              = SysProcessType, &
   SYSGETCOLLATE.56               = SysGetCollate, &
   SYSNATIONALLANGUAGECOMPARE.57  = SysNationalLanguageCompare, &
   SYSMAPCASE.58                  = SysMapCase, &
   SYSSETPROCESSCODEPAGE.59       = SysSetProcessCodePage, &
   SYSQUERYPROCESSCODEPAGE.60     = SysQueryProcessCodePage, &
   SYSADDREXXMACRO.61             = SysAddRexxMacro, &
   SYSDROPREXXMACRO.62            = SysDropRexxMacro, &
   SYSREORDERREXXMACRO.63         = SysReorderRexxMacro, &
   SYSQUERYREXXMACRO.64           = SysQueryRexxMacro, &
   SYSCLEARREXXMACROSPACE.65      = SysClearRexxMacroSpace, &
   SYSLOADREXXMACROSPACE.66       = SysLoadRexxMacroSpace, &
   SYSSAVEREXXMACROSPACE.67       = SysSaveRexxMacroSpace, &
   SYSSHUTDOWNSYSTEM.68           = SysShutDownSystem, &
   SYSSWITCHSESION.69             = SysSwitchSession, &
#  Undocumented returns INVALID_ROUTINE
   SYSLOADLIBRARY.70              = SysLoadLibrary, &
#  Undocumented returns INVALID_ROUTINE
   SYSDROPLIBRARY.71              = SysDropLibrary, &
   SYSELAPSEDTIME.72              = SysElapsedTime, &
   SYSWAITFORSHELL.73             = SysWaitForShell, &
   SYSQUERYSWITCHLIST.74          = SysQuerySwitchList, &
   SYSQUERYEXTLIBPATH.75          = SysQueryExtLIBPATH, &
   SYSSETEXTLIBPATH.76            = SysSetExtLIBPATH, &
   SYSDUMPVARIABLES.77            = SysDumpVariables

DEST         = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
