/*
 *   (c) osFree project
 *   author valerius, 2007, Oct 12
 *   Skips extra zeroes between _TEXT16 and
 *   _TEXT segments in stage0 raw binary
 */

parse arg args

if words(args) \= 4 then
   signal usage

parse var args var incfile file outfile

/* read STAGE0_BASE from include file */
base = readbase(incfile, var)

/* read _TEXT16 segment size from map file */
_text16_size = seg16_size(file)

if _text16_size > 0 then do

  /* read _TEXT16 segment */
  buf = charin(file, 1, _text16_size)
  call charout outfile, buf

end

/* binary size */
fsize = stream(file, 'c', 'query size')

/* read DGROUP segment */
buf = charin(file, _text16_size + base + 1, fsize - base - _text16_size)
call charout outfile, buf, _text16_size + 1


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
readbase: procedure
file = arg(1)
var  = arg(2)

line = getline(var, file)
line = strip(word(line, 3))
line = strip(line, 'T', 'h')

p = pos('0x', line)
if p = 1 then
  line = delstr(line, 1,  2)

if line = '' then
   return 0

return x2d(line)
/* ------------------------ */
seg16_size: procedure
file = arg(1)

p = lastpos('.', file)
if p > 0 then
   file = substr(file, 1, p) || 'wmp'

line = getline('_TEXT16 ', file)

if line = '' then
   return 0

return x2d(strip(word(line, 5)))
/* ------------------------ */
usage:

say 'Usage:'
say
say 'ripzeroes <variable> <incfile> <infile> <outfile>'
say

exit -1
/* ------------------------ */
