#!/usr/bin/env regina
parse arg f
call rxfuncadd 'sysloadfuncs','RexxUtil','sysloadfuncs'

call sysloadfuncs
say 'SysDriveInfo' SysDriveInfo(f)
