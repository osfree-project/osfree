/* test of the regina utilities */
call rxfuncadd 'sysloadfuncs','RexxUtil','sysloadfuncs'
call sysloadfuncs

say 'SysWINVer' SysWinVer()
say 'SysUtilVersion' SysUtilVersion()
Call SysCurState'off'
Call SysCurState 'on'
