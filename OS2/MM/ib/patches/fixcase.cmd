/* rexx */

parse arg dir

call directory dir

call rename 'CAMERA.ICO',      'camera.ico'
call rename 'CROSS.ICO',       'cross.ico'
call rename 'IMAGE.C',         'image.c'
call rename 'IMAGE.C',         'image.c'
call rename 'IMAGE.DEF',       'image.def'
call rename 'IMAGE.DLG',       'image.dlg'
call rename 'IMAGE.H',         'image.h'
call rename 'IMAGE.ICO',       'image.ico'
call rename 'IMAGE.RC',        'image.rc'
call rename 'IMAGE2.C',        'image2.c'
call rename 'MAKEFILE',        'makefile'


exit 0
/* ------------------------------------------ */
rename: procedure
src = arg(1)
dst = arg(2)

parse source os .

if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32' | os = 'WIN64'
then MV = 'move'
else MV = 'mv'

ret = stream(src, 'c', 'query exists')

if ret \= '' then do
    MV' 'src' 'dst'1'
    MV' 'dst'1 'dst
end

return
/* ------------------------------------------ */
