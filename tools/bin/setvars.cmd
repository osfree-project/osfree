/**/

parse arg dir nestlevel

env = 'ENVIRONMENT'
mydir = dir

dir = translate(dir, '\', '/')
dir = strip(dir, 'T', '\')

reldir = ''
do while nestlevel > 0
  p = lastpos('\', dir)
  if p > 0 then do
    d   = substr(dir, p + 1)
    reldir = d || '\' || reldir
    dir = delstr(dir, p)
    nestlevel = nestlevel - 1
  end
  else
    leave
end

rootdir = dir
if pos('\', rootdir) \= length(rootdir)
then rootdir = rootdir || '\'


call value 'RELDIR', reldir, env
call value 'MYDIR',  mydir, env

'set RELDIR='reldir
'set MYDIR='mydir
'set ROOTDIR='rootdir


say reldir
say mydir
say rootdir
