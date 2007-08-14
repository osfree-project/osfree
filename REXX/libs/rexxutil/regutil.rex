/* test of the regina utilities */
call rxfuncadd 'sysloadfuncs','RexxUtil','sysloadfuncs'

call sysloadfuncs

say 'SysCls' SysCls()
say 'SysAddRexxMacro' SysAddRexxMacro(x,x)
say 'SysClearRexxMacroSpace' SysClearRexxMacroSpace()
say 'SysDropRexxMacro' SysDropRexxMacro()
say 'SysLoadRexxMacroSpace' SysLoadRexxMacroSpace()
say 'SysQueryRexxMacro' SysQueryRexxMacro()
say 'SysReorderRexxMacro' SysReorderRexxMacro()
say 'SysSaveRexxMacroSpace' SysSaveRexxMacroSpace()
say 'SysCurPos' SysCurPos()
say 'SysCurState' SysCurState('on')
say 'press a key'
say 'SysGetKey' SysGetKey('n')
say 'SysTextScreenRead' SysTextScreenRead(1,1,10)
say 'SysTextScreenSize' SysTextScreenSize()

/*
say 'SysCloseEventSem' SysCloseEventSem()
say 'SysCloseMutexSem' SysCloseMutexSem()
say 'SysCreateEventSem' SysCreateEventSem()
say 'SysCreateMutexSem' SysCreateMutexSem()
say 'SysOpenEventSem' SysOpenEventSem()
say 'SysOpenMutexSem' SysOpenMutexSem()
say 'SysPostEventSem' SysPostEventSem()
say 'SysPulseEventSem' SysPulseEventSem()
say 'SysReleaseMutexSem' SysReleaseMutexSem()
say 'SysRequestMutexSem' SysRequestMutexSem()
say 'SysResetEventSem' SysResetEventSem()
say 'SysWaitEventSem' SysWaitEventSem() */
say 'SysBootDrive' SysBootDrive()
say 'SysWINVer' SysWINVer()
say 'SysDriveInfo' SysDriveInfo('C:\')
say 'SysDriveMap' SysDriveMap()
say 'SysSetPriority' SysSetPriority()
say 'SysSleep' SysSleep(1.1)
say 'SysSwitchSession' SysSwitchSession()
say 'SysSystemDirectory' SysSystemDirectory()
say 'SysVolumeLabel' SysVolumeLabel()

call sysfilesearch 'sysfilesearch','regfilesys.c','NOODLE.'
do i = 1 to noodle.0
  say noodle.i
  end

call sysfilesearch 'sysfilesearch','regfilesys.c','NOODLE.', 'c'
do i = 1 to noodle.0
  say noodle.i
  end

call sysfilesearch 'sysfilesearch','regfilesys.c','NOODLE.', 'cn'
do i = 1 to noodle.0
  say noodle.i
  end

call sysfilesearch 'sysfilesearch','regfilesys.c','NOODLE.', 'n'
do i = 1 to noodle.0
  say noodle.i
  end

say 'SysDropFuncs' SysDropFuncs()
