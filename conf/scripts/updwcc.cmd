/* Update _wcc.{cmd,sh}
 *
 * Usage:
 *    cd %ROOT%
 *    find . -type f -name _wcc.sh -exec /usr/local/bin/rexx ./x.cmd {} \;
 *    find . -type f -name _wcc.sh -exec chmod 755 {} \;
 *    find . -type f -name _wcc.sh -exec git add {} \;
 *
 */

parse arg fn sep

if sep = '' sep = '\'
fn  = translate(fn, '/', sep)
l   = levels(fn)

if sep = '/' then ext = 'sh' else ext = 'cmd'
'sed -e ''s,\@RT\@,'l','' conf/scripts/_wcc.'ext'-template >'fn

exit
/* ----------------------------------- */
levels: procedure expose sep
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
    dir = dir || sep'..'
  end
end

return dir
/* ----------------------------------- */
