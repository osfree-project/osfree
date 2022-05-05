parse arg f
call rxfuncadd 'sysloadfuncs','RexxUtil','sysloadfuncs'

call sysloadfuncs
say 'SysDriveInfo' SysDriveInfo(f)
