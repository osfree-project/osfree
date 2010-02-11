#!/usr/bin/rexx
/**/
parse source . . src
p = lastpos('/', translate(src, '/', '\'))
path = delstr(src, p)
basename = substr(src, p + 1)
p = lastpos('.', basename)
if p > 0 then basename = delstr(basename, p)
user = substr(basename, pos('-', basename) + 1)
curdir = directory()
call directory path
call SETENV user || '.conf'
call directory curdir

say 'New shell started'
interpret '"'||address()||'"'
