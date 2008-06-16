# $Id: makefile,v 1.3 2004/08/16 04:51:08 prokushev Exp $

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS = 1
DEBUG = 0

!ifeq DEBUG 1
OPT   = -d3 -of -od
!else
OPT   = -d0 -oteanx
!endif

CLEANMASK = *.obj *.map *.err *.wmp *.lnk *.sym
DIR = $(%ROOT)$(SEP)files$(SEP)os2$(SEP)dll
PROJ = rexxutil
DESC = REXX Utilities API
ADD_COPT = -i=. -bd -bm -wx -s -mf $(OPT) -6s -fp6 -bd

OBJS = rexxfuncs.$(O) sysfile.$(O)   sysmiscfile.$(O) systextscreen.$(O) sysversion.$(O)  &
       sysdrive.$(O)  syseautil.$(O) sysobjects.$(O)  sysprocess.$(O)    syslang.$(O)     &
       sysmacro.$(O)  sysstem.$(O)   sysmisc.$(O)     sysmutexsem.$(O)   syseventsem.$(O) &
       sysmath.$(O)   helperfuncs.$(O) rxmessage.$(O)

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).dll $(DIR)$(SEP)$(PROJ).sym

$(DIR)$(SEP)$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(DIR)$(SEP)$(PROJ).dll
 @%append $^&.lnk system    os2v2 dll initinstance terminstance
 @%append $^&.lnk option    ManyAutoData
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk export
 @%append $^&.lnk   SYSCLS.1                       = SysCls,
 @%append $^&.lnk   SYSCURPOS.2                    = SysCurPos,
 @%append $^&.lnk   SYSCURSTATE.3                  = SysCurState,
 @%append $^&.lnk   SYSDRIVEINFO.4                 = SysDriveInfo,
 @%append $^&.lnk   SYSDRIVEMAP.5                  = SysDriveMap,
 @%append $^&.lnk   SYSDROPFUNCS.6                 = SysDropFuncs,
 @%append $^&.lnk   SYSFILEDELETE.7                = SysFileDelete,
 @%append $^&.lnk   SYSFILESEARCH.8                = SysFileSearch,
 @%append $^&.lnk   SYSFILETREE.9                  = SysFileTree,
 @%append $^&.lnk   SYSGETKEY.10                   = SysGetKey,
 @%append $^&.lnk   SYSGETMESSAGE.11               = SysGetMessage,
 @%append $^&.lnk   SYSINI.12                      = SysIni,
 @%append $^&.lnk   SYSLOADFUNCS.13                = SysLoadFuncs,
 @%append $^&.lnk   SYSMKDIR.14                    = SysMkDir,
 @%append $^&.lnk   SYSOS2VER.15                   = SysOS2Ver,
 @%append $^&.lnk   SYSRMDIR.16                    = SysRmDir,
 @%append $^&.lnk   SYSSEARCHPATH.17               = SysSearchPath,
 @%append $^&.lnk   SYSSLEEP.18                    = SysSleep,
 @%append $^&.lnk   SYSTEMPFILENAME.19             = SysTempFileName,
 @%append $^&.lnk   SYSTEXTSCREENREAD.20           = SysTextScreenRead,
 @%append $^&.lnk   SYSTEXTSCREENSIZE.21           = SysTextScreenSize,
 @%append $^&.lnk   SYSGETEA.22                    = SysGetEA,
 @%append $^&.lnk   SYSPUTEA.23                    = SysPutEA,
 @%append $^&.lnk   SYSWAITNAMEDPIPE.24            = SysWaitNamedPipe,
 @%append $^&.lnk   SYSSETICON.25                  = SysSetIcon,
 @%append $^&.lnk   SYSREGISTEROBJECTCLASS.26      = SysRegisterObjectClass,
 @%append $^&.lnk   SYSDEREGISTEROBJECTCLASS.27    = SysDeregisterObjectClass,
 @%append $^&.lnk   SYSCREATEOBJECT.28             = SysCreateObject,
 @%append $^&.lnk   SYSQUERYCLASSLIST.29           = SysQueryClassList,
 @%append $^&.lnk   SYSDESTROYOBJECT.30            = SysDestroyObject,
 @%append $^&.lnk   SYSSETOBJECTDATA.31            = SysSetObjectData,
 @%append $^&.lnk   SYSSAVEOBJECT.32               = SysSaveObject,
 @%append $^&.lnk   SYSOPENOBJECT.33               = SysOpenObject,
 @%append $^&.lnk   SYSMOVEOBJECT.34               = SysMoveObject,
 @%append $^&.lnk   SYSCOPYOBJECT.35               = SysCopyObject,
 @%append $^&.lnk   SYSCREATESHADOW.36             = SysCreateShadow,
 @%append $^&.lnk   SYSBOOTDRIVE.37                = SysBootDrive,
 @%append $^&.lnk   SYSQUERYEALIST.38              = SysQueryEAList,
 @%append $^&.lnk   SYSWILDCARD.39                 = SysWildCard,
 @%append $^&.lnk   SYSFILESYSTEMTYPE.40           = SysFileSystemType,
 @%append $^&.lnk   SYSADDFILEHANDLE.41            = SysAddFileHandle,
 @%append $^&.lnk   SYSSETFILEHANDLE.42            = SysSetFileHandle,
 @%append $^&.lnk   SYSCREATEMUTEXSEM.43           = SysCreateMutexSem,
 @%append $^&.lnk   SYSOPENMUTEXSEM.44             = SysOpenMutexSem,
 @%append $^&.lnk   SYSCLOSEMUTEXSEM.45            = SysCloseMutexSem,
 @%append $^&.lnk   SYSREQUESTMUTEXSEM.46          = SysRequestMutexSem,
 @%append $^&.lnk   SYSRELEASEMUTEXSEM.47          = SysReleaseMutexSem,
 @%append $^&.lnk   SYSCREATEEVENTSEM.48           = SysCreateEventSem,
 @%append $^&.lnk   SYSOPENEVENTSEM.49             = SysOpenEventSem,
 @%append $^&.lnk   SYSCLOSEEVENTSEM.50            = SysCloseEventSem,
 @%append $^&.lnk   SYSRESETEVENTSEM.51            = SysResetEventSem,
 @%append $^&.lnk   SYSPOSTEVENTSEM.52             = SysPostEventSem,
 @%append $^&.lnk   SYSWAITEVENTSEM.53             = SysWaitEventSem,
 @%append $^&.lnk   SYSSETPRIORITY.54              = SysSetPriority,
 @%append $^&.lnk   SYSPROCESSTYPE.55              = SysProcessType,
 @%append $^&.lnk   SYSGETCOLLATE.56               = SysGetCollate,
 @%append $^&.lnk   SYSNATIONALLANGUAGECOMPARE.57  = SysNationalLanguageCompare,
 @%append $^&.lnk   SYSMAPCASE.58                  = SysMapCase,
 @%append $^&.lnk   SYSSETPROCESSCODEPAGE.59       = SysSetProcessCodePage,
 @%append $^&.lnk   SYSQUERYPROCESSCODEPAGE.60     = SysQueryProcessCodePage,
 @%append $^&.lnk   SYSADDREXXMACRO.61             = SysAddRexxMacro,
 @%append $^&.lnk   SYSDROPREXXMACRO.62            = SysDropRexxMacro,
 @%append $^&.lnk   SYSREORDERREXXMACRO.63         = SysReorderRexxMacro,
 @%append $^&.lnk   SYSQUERYREXXMACRO.64           = SysQueryRexxMacro,
 @%append $^&.lnk   SYSCLEARREXXMACROSPACE.65      = SysClearRexxMacroSpace,
 @%append $^&.lnk   SYSLOADREXXMACROSPACE.66       = SysLoadRexxMacroSpace,
 @%append $^&.lnk   SYSSAVEREXXMACROSPACE.67       = SysSaveRexxMacroSpace,
 @%append $^&.lnk   SYSSHUTDOWNSYSTEM.68           = SysShutDownSystem,
 @%append $^&.lnk   SYSSWITCHSESION.69             = SysSwitchSession,
 @%append $^&.lnk   SYSLOADLIBRARY.70              = SysLoadLibrary,  # Undocumented returns INVALID_ROUTINE
 @%append $^&.lnk   SYSDROPLIBRARY.71              = SysDropLibrary,  # Undocumented returns INVALID_ROUTINE
 @%append $^&.lnk   SYSELAPSEDTIME.72              = SysElapsedTime,
 @%append $^&.lnk   SYSWAITFORSHELL.73             = SysWaitForShell,
 @%append $^&.lnk   SYSQUERYSWITCHLIST.74          = SysQuerySwitchList,
 @%append $^&.lnk   SYSQUERYEXTLIBPATH.75          = SysQueryExtLIBPATH,
 @%append $^&.lnk   SYSSETEXTLIBPATH.76            = SysSetExtLIBPATH,
 @%append $^&.lnk   SYSDUMPVARIABLES.77            = SysDumpVariables,
 @%append $^&.lnk   SYSSETFILEDATETIME.78          = SysSetFileDateTime,
 @%append $^&.lnk   SYSGETFILEDATETIME.79          = SysGetFileDateTime,
 @%append $^&.lnk   SYSSTEMSORT.80                 = SysStemSort,
 @%append $^&.lnk   SYSSTEMDELETE.81               = SysStemDelete,
 @%append $^&.lnk   SYSSTEMINSERT.82               = SysStemInsert,
 @%append $^&.lnk   SYSSTEMCOPY.83                 = SysStemCopy,
 @%append $^&.lnk   SYSVERSION.84                  = SysVersion,
 @%append $^&.lnk   SYSUTILVERSION.85              = SysUtilVersion,
# Additional functions
 @%append $^&.lnk   RXMESSAGEBOX.86                = RxMessageBox,
 @%append $^&.lnk   SYSREPLACEOBJECT.87            = SysReplaceObject,
# Math functions
 @%append $^&.lnk   SYSPI                          = SysPi,
 @%append $^&.lnk   SYSSQRT                        = SysSqrt,
 @%append $^&.lnk   SYSEXP                         = SysExp,
 @%append $^&.lnk   SYSLOG                         = SysLog,
 @%append $^&.lnk   SYSLOG10                       = SysLog10,
 @%append $^&.lnk   SYSSINH                        = SysSinH,
 @%append $^&.lnk   SYSCOSH                        = SysCosH,
 @%append $^&.lnk   SYSTANH                        = SysTanH,
 @%append $^&.lnk   SYSPOWER                       = SysPower,
 @%append $^&.lnk   SYSSIN                         = SysSin,
 @%append $^&.lnk   SYSCOS                         = SysCos,
 @%append $^&.lnk   SYSTAN                         = SysTan,
 @%append $^&.lnk   SYSCOTAN                       = SysCotan,
 @%append $^&.lnk   SYSARCSIN                      = SysArcSin,
 @%append $^&.lnk   SYSARCCOS                      = SysArcCos,
 @%append $^&.lnk   SYSARCTAN                      = SysArcTan
 $(ADDFILES_CMD)
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

$(DIR)$(SEP)$(PROJ).sym: $(PROJ).sym
 $(CP) *.sym $(DIR)

$(PROJ).sym: $(PROJ).map

$(PROJ).map: $(PROJ).wmp
 
# Clean all
clean: .symbolic
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
