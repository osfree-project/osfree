rcc = rxfuncadd('sysloadfuncs','rexxutil','sysloadfuncs')

if rcc then do
 exit 1
 end

say 'sysloadfuncs:' sysloadfuncs()

say 'sysfiletree:' sysfiletree(value('HOME',,'ENVIRONMENT')'/*.c', 'FOO.', 'FST')
do i = 1 to foo.0
   say foo.i
   end
