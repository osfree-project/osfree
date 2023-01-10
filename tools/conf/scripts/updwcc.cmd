/* Update _wcc.{cmd,sh}
 *
 * Usage:
 *    cd %ROOT%
 *    find . -type f -name _wcc.sh -exec /usr/local/bin/rexx ./tools/conf/scripts/updwcc.cmd {} / \;
 *    find . -type f -name _wcc.sh -exec chmod 755 {} \;
 *    find . -type f -name _wcc.sh -exec git add {} \;
 *
 */

parse arg fn sep

verbose = ''
if sep = ''  then sep = '\'
if sep = '\' then verbose = '@'
if sep = '\' then sep2 = sep || sep; else sep2 = sep
root = value('ROOT',, 'ENVIRONMENT')
if root = '' then root = './'
fn1  = translate(fn, '/', '\')
fn2 = substr(fn1, length(root))
l   = levels(fn2)
p   = lastpos('.', fn)
ext = substr(fn, p + 1)

verbose'sed -e "s,\@RT\@,'l'," 'root'tools/conf/scripts/_wcc.'ext'-template >'fn

exit
/* ----------------------------------- */
levels: procedure expose sep2
parse arg fn

count = 0

do forever
  parse value fn with x '/' fn
  if fn <> '' then
    count = count + 1
  else
    leave
end

dir = '..'

select
  when count = 1 then dir = '.'
  when count = 2 then dir = '..'
  otherwise do i = 3 to count
    dir = dir || sep2'..'
  end
end

return dir
/* ----------------------------------- */
