/* makewpi049.cmd - create a German WarpIN NLS package for XWorkplace */

/* change last two characters to your language code */
/* use the current version number                   */
wpistem = 'xwp-1-1-0_nls-de'

/* use the current version number as in '0<major>:<minor><revision>:00' */
filetime = '01:10:00'

/* change the three digits to your country code */
countrycode = '049'

/* no configurable parts below here */

wisfile = 'nls'||countrycode||'.wis'
packages.0 = 2
packages.1.id = 1000 + countrycode
packages.1.dir = countrycode
packages.2.id = 2000 + countrycode
packages.2.dir = 'inf'||countrycode
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs
parse arg reldir
if reldir = '' then do
  say 'Usage: makewpi <release directory>'
  return
end
makedir = directory()
reldir = directory(reldir)
if reldir = '' then do
  say 'Invalid release directory'
  return
end
warpindir = SysIni(USER, 'WarpIN', 'Path')
if warpindir = 'ERROR:' | warpindir = '' then do
  say 'WarpIN is not installed correctly'
  return
end
call directory warpindir
wic1 = '@wic' reldir'\'wpistem '-a'
wic2 = ''
do i = 1 to packages.0
  wic2 = wic2 packages.i.id '-r -c'reldir'\'packages.i.dir '*'
  call setfiletime reldir'\'packages.i.dir
end
wic3 = '-u -s' makedir'\'wisfile
call SysFileDelete reldir'\'wpistem'.exe'
wic1 wic2 wic3
call directory makedir
return

setfiletime: procedure expose filetime makedir
  parse arg pkgdir
  call SysFileTree pkgdir'\*', 'stem', 'FOS'
  filedate = date('S')
  filedate = left(filedate,4)'-'substr(filedate,5,2)'-'substr(filedate,7,2)
  do i = 1 to stem.0
    call SysSetFileDateTime stem.i, filedate, filetime
    '@'makedir'\..\tools\setftime' stem.i
  end
  return
