/*
 *  build directory hierarchy
 *  (c) osFree project, 2008
 *  Input: absolute or relative
 *  directory path
 *  Result: all path components
 *  are made in sequence
 */

parse arg dir
parse source os .
parse version rexxname rexxver releasedate

if os = 'OS/2' | os = 'DOS' | os = 'WINDOWS' |,
   os = 'WINNT' | os = 'WIN32'
then do
  sep = '\'
  nul = 'nul'
end
else do
  sep = '/'
  nul = '/dev/null'
end

curdir = directory()
dir = translate(dir, '\', '/')
path = dir

absolute = 0
drv = ''

if substr(dir, 2, 1) = ':' & substr(dir, 3, 1) = sep
then do
  absolute = 1
  drv = substr(dir, 1, 2)
end

if substr(dir, 1, 1) = '\'
then absolute = 1

/* if the path is absolute, then go root */
if absolute then do
  if drv \= '' then drv /* change drive */
  call directory sep
end

do while path \= ''
  parse value path with dir '\' path
  if pos(':', dir) = 2 & length(dir) = 2
  then
    iterate
  
  if dir = '' then iterate

  cdir=directory()
  ndir=directory(dir)
  if ((ndir = cdir)|(ndir='')) /* ooREXX 4.0 6.03 returns current dir instead null*/
  then do
    'mkdir ' || dir
    call directory dir
  end
end

call directory curdir

exit 0
