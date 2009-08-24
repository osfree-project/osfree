/* this program tests the behaviour of reaching EOF using LINEIN */

Parse Source os . fn
do while lines(fn) > 0
   say '1-->' linein(fn)
end
call stream fn, 'C', 'CLOSE'

signal on notready name eof
do forever
   say '2-->' linein(fn)
end
Say "Shouldn't get here!"
Return
eof:
say 'Got NOTREADY from' sigl 'This is correct!'

file='eoftest.dat'
if os = 'UNIX' Then
call charout file,'a'|| '0a'x || '1a'x,1
else
call charout file,'a'|| '0d0a'x || '1a'x,1
call stream file, 'c', 'close'
call on notready name linein_err
rc=0
do while rc=0
  say c2x(linein(file))
end
say 'Should get here! Returning...'
return
linein_err:
say 'got NOTREADY from line' sigl 'This is correct!'
rc=1
return
/* last line */
