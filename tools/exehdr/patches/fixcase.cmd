/* rexx */

SEP = getsep()

parse arg dir

call directory dir
call directory 'msg'

'mkdir exh'
'mkdir exh' || SEP || 'en'
'mkdir exh' || SEP || 'pl'
'mkdir exh' || SEP || 'ru'

call cpy 'exh.txt',   'exh' || SEP || 'en'
call cpy 'exh.txt',   'exh' || SEP || 'pl'
call cpy 'exh.txt',   'exh' || SEP || 'ru'

'mkdir exhh'
'mkdir exhh' || SEP || 'en'
'mkdir exhh' || SEP || 'pl'
'mkdir exhh' || SEP || 'ru'

call cpy 'exhh.txt',   'exhh' || SEP || 'en'
call cpy 'exhh.txt',   'exhh' || SEP || 'pl'
call cpy 'exhh.txt',   'exhh' || SEP || 'ru'

call delete 'exh.txt'
call delete 'exhh.txt'

exit 0
/* ------------------------------------------ */
getsep: procedure

parse source os .

if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32' | os = 'WIN64'
then SEP = '\'
else SEP = '/'

return SEP
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
cpy: procedure
src = arg(1)
dst = arg(2)

parse source os .

if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32' | os = 'WIN64'
then CP = 'copy'
else CP = 'cp'

ret = stream(src, 'c', 'query exists')

if ret \= '' then do
    CP' 'src' 'dst
end

return
/* ------------------------------------------ */
delete: procedure
trg = arg(1)

parse source os .

if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32' | os = 'WIN64'
then RM = 'del'
else RM = 'rm -rf'

ret = stream(trg, 'c', 'query exists')

if ret \= '' then do
    RM' 'trg
end

return
/* ------------------------------------------ */
