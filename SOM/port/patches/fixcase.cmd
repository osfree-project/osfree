/* rexx */
parse arg dir

parse source os .

'dos2unix include/win32/link386.h'
'dos2unix sc/src/sc.c'
'dos2unix somipc/src/rhbsc.cpp'
'dos2unix somtklib/win32/somtklib.mak'

if os = 'LINUX' | os = 'UNIX' then do
'mkdir -p somcorb2/linux'
'mkdir -p somstars/linux'
'mkdir -p somhh/linux'
'mkdir -p somxh/linux'

'touch somcorb2/linux/somcorba'
'touch somstars/linux/somstars'
'touch somhh/linux/somhh'
'touch somxh/linux/somxh'

'chmod 755 somcorb2/linux/somcorba'
'chmod 755 somstars/linux/somstars'
'chmod 755 somhh/linux/somhh'
'chmod 755 somxh/linux/somxh'
end

exit 0
