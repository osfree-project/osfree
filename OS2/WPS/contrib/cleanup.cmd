/* Cleanup XWorkplace */
signal on halt
parse upper arg y2all bld_typ lang .
if bld_typ = '' then do
  call charout , 'Build type [FULL/lite]: '
  parse linein bld_typ .
  if bld_typ = '' then
    bld_typ = 'FULL'
end
if lang = '' then do
  call charout , 'Language code [001]: '
  parse linein lang .
  if lang = '' then
    lang = '001'
end

/* Compiled XWorkplace files */
say 'Do you wish to delete all .OBJ, .RES, .EXE, .DLL, .MAP, .SYM and .TMF files in'
call charout , 'the source directories [Y/N] ? '
if y2all = 'Y' then say 'Y'; else parse upper linein yn .
if y2all = 'Y' | yn = 'Y' then do
  call deletefiles 'bin\'||bld_typ||'\*.obj'
  call deletefiles 'bin\'||bld_typ||'\*.res'
  call deletefiles 'bin\'||bld_typ||'\dll_mt\*.obj'
  call deletefiles 'bin\'||bld_typ||'\dll_mt\*.res'
  call deletefiles 'bin\'||bld_typ||'\dll_subs\*.obj'
  call deletefiles 'bin\'||bld_typ||'\exe_mt\*.obj'
  call deletefiles 'bin\'||bld_typ||'\exe_mt\*.res'
  call deletefiles 'bin\'||bld_typ||'\exe_st\*.obj'
  call deletefiles 'bin\'||bld_typ||'\modules\*.exe'
  call deletefiles 'bin\'||bld_typ||'\modules\*.dll'
  call deletefiles 'bin\'||bld_typ||'\modules\*.map'
  call deletefiles 'bin\'||bld_typ||'\modules\*.sym'
  call deletefiles 'bin\'||bld_typ||'\modules\*.tmf'
  call deletefiles 'bin\'||bld_typ||'\widgets\*.obj'
end

/* LIB files */
call charout , 'Do you wish to delete the LIB files [Y/N] ? '
if y2all = 'Y' then say 'Y'; else parse upper linein yn .
if y2all = 'Y' | yn = 'Y' then do
  call deletefiles 'bin\'||bld_typ||'\*.lib'
  call deletefiles 'bin\'||bld_typ||'\exe_mt\*.lib'
end

/* INF/HLP files */
call charout , 'Do you wish to delete the INF/HLP files [Y/N] ? '
if y2all = 'Y' then say 'Y'; else parse upper linein yn .
if y2all = 'Y' | yn = 'Y' then do
  call deletefiles 'bin\'||bld_typ||'\modules\*.inf'
  call deletefiles 'bin\'||bld_typ||'\modules\*.hlp'
end

/* IPF source files */
call charout , 'Do you wish to delete the IPF source files [Y/N] ? '
if y2all = 'Y' then say 'Y'; else parse upper linein yn .
if y2all = 'Y' | yn = 'Y' then do
  call deletefiles 'bin\'||bld_typ||'\modules\*.ipf'
  call deletefiles lang||'\inf.'||lang||'\*.bmp'
  call deletefiles lang||'\xwphelp2\img\*.bmp'
end

/* SOM headers */
call charout , 'Do you wish to delete all the SC-created .DEF, .IH and .H files [Y/N] ? '
if y2all = 'Y' then say 'Y'; else parse upper linein yn .
if y2all = 'Y' | yn = 'Y' then do
  call deletefiles 'idl\*.def'
  call deletefiles 'include\classes\*.h'
  call deletefiles 'include\classes\*.ih'
  call deletefiles 'include\classes\wps\*.h'
  call deletefiles 'include\classes\wps\*.ih'
end
return

halt:
  say
  return

deletefiles: procedure
  arg name
  '@if exist' name 'del' name
  return
