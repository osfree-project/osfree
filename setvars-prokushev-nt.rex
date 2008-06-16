/**/
'@echo off'
parse source . . src
p = lastpos('/', translate(src, '/', '\'))
path = delstr(src, p)
basename = substr(src, p + 1)
p = lastpos('.', basename)
if p > 0 then basename = delstr(basename, p)
user = substr(basename, pos('-', basename) + 1)
curdir = directory()
call directory path
call setenv user || '.conf'
call directory curdir

cmd