/* Change directory with trailing slash
 * and (optional) beginning with a drive
 * letter
 * (c) osFree project
 * valerius, 2008 Jul
 */

parse arg dir
parse source os .

dir = strip(dir, 'B')

if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32' | os = 'WIN64'
then do
  sep = '\'
  '@echo off'
end; else
  sep = '/'

dir = translate(dir, '\', '/')
drv = ''

do forever
  p = pos('\\', dir)
  if p > 0
  then
    dir = delstr(dir, p, 1)
  else
    leave
end

parse value dir with drv ':' dir

if length(drv) = 1 then do
  /* change drive first */
  '@' || drv || ':'
end; else; do
  dir = drv
  drv = ''
end

dir = strip(dir, 'T', '\')
if sep = '/' then dir = translate(dir, '/', '\')
'cd ' || dir
