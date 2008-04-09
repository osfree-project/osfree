/*
 *   (c) osFree project
 *   author valerius, 2007, Oct 12
 *   Skips extra zeroes between _TEXT16 and
 *   _TEXT segments in stage0 raw binary
 */

Parse Source os src .

parse arg args

p = words(args)
if p < 3 | p > 4 then
   signal usage

parse var args file var incfile shift

shift = strip(shift)
p = pos('0x', shift)
if p > 0 then shift = x2d(delstr(shift, 1, 2))
if shift = '' then shift = 0

add = 0

/* read STAGE0_BASE from include file */
base = readbase(incfile, var)

sout = ''

/* read _TEXT16 segment size from map file */
_text16_size = seg16_size(file)

if _text16_size > 0 then do

  /* read _TEXT16 segment */
  buf = charin(file, 1, _text16_size)
  sout = sout || buf

end

/* workaround of zerosize return. Most probably required only for network disks. */
if os = 'WIN32' then sleep(10)

/* binary size */
fsize = stream(file, 'C', 'QUERY SIZE')

/*
call charout stderr, fsize || '10 13'x
call charout stderr, base || '10 13'x
call charout stderr, _text16_size || '10 13'x
 */

/* read DGROUP segment */
buf = charin(file, _text16_size + base + 1, fsize - base - _text16_size)
sout = sout || buf

/* Pad file to a multiple of 4 bytes */
sz = fsize - base

n = 0
do while (sz + n) // 4 > 0
  n = n + 1
  sout = sout || '00'x
end


call charout , sout


exit 0
/* ------------------------ */
getline: procedure
s = arg(1)
file = arg(2)
/* get a line from file,
   beginning from string s */

call stream file, 'c', 'open read'

do while lines(file) > 0
  line = strip(linein(file))
  if abbrev(line, s, length(s)) then
     leave
end

call stream file, 'c', 'close'

if lines(file) = 0 then
   return ''

return line
/* ------------------------ */
readbase: procedure expose shift
file = arg(1)
var  = arg(2)

line = getline(var, file)
p = pos(';', line)
if p > 0 then line = strip(delstr(line, p))
p = pos('equ', line)
if p > 0 then line = strip(substr(line, p + 4))

line = strip(line, 'L' ,'(')
line = strip(line, 'T' ,')')

/* Delete spaces */
do forever
  p = pos(' ', line)
  if p <= 0
  then
    leave
  else
    line = delstr(line, p, 1)
end

p = pos('+SHIFT', line)
if p > 0 then do
  add = 1
  line = strip(delstr(line, p))
end

p = pos('0x', line)

if p = 1 then do
  line = delstr(line, 1,  2)
  if line = '' then
    return 0
  line = x2d(line)
end

if add = 1 then line = line + shift


return line
/* ------------------------ */
seg16_size: procedure
file = arg(1)

p = lastpos('.', file)
if p > 0 then do
  l    = substr(file, p + 1, 1)
  if l = 'b'  then l = ''
  file = substr(file, 1, p - 1) || l || '.wmp'
end

line = getline('_TEXT16 ', file)

if line = '' then
   return 0

return x2d(strip(word(line, 5)))
/* ------------------------ */
usage:

say 'Usage:'
say
say 'ripzeroes <infile> <variable> <incfile> [<shift factor>] ><outfile>'
say

exit -1
/* ------------------------ */
