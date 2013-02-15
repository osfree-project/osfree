/* Change directory with trailing slash
 * and (optional) beginning with a drive
 * letter
 * (c) osFree project
 * valerius, 2008 Jul
 */

parse arg dir
parse source os .


if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32'
then
  sep = '\'
else
  sep = '/'
  
if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32'
then do
   dir = translate(dir, '\', '/') /* change / to \  */
end
drv = ''

do forever
  p = pos('\\', dir)
  if p > 0
  then
    dir = delstr(dir, p, 1)
  else
    leave
end

if pos('\', dir) > 0
then parse value dir with drv '\' dir

if length(drv) = 2 & substr(drv, 2, 1) = ':'
then do
  drv /* change drive first */
  dir = sep || dir
end
dir = strip(dir, 'T', '\')
call directory(dir)
say '  cd 'dir
