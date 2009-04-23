/**/
'@echo off'

d=directory()
drive=substr(d, 1,1)

parse source . . src
p = lastpos('/', translate(src, '/', '\'))
path = delstr(src, p)
basename = substr(src, p + 1)
p = lastpos('.', basename)
if p > 0 then basename = delstr(basename, p)
user = substr(basename, pos('-', basename) + 1)
curdir = directory()
call directory path
f=user || '.conf'
'@del '||f
rc=lineout(f, 'watcom  = "'||drive||':\osfree\watcom"')
rc=lineout(f,'root    = "'||drive||':\osFree\osfree"')
rc=lineout(f,'fppath  = "'||drive||':\osfree\fpc\2.2.0\bin\i386-win32"')
rc=lineout(f,'shell   = "cmd.exe"')
rc=lineout(f,'rexx    = "'||drive||':\osfree\regina\regina.exe"')
rc=lineout(f,'rexx_path    = "'||drive||':\osfree\regina\"')
rc=lineout(f,'serverenv = "win32"');
rc=stream(f,'c','close')
call setenv f
call directory curdir

cmd