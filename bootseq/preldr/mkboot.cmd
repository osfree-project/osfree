/*
 *  Make boot block from preldr0 and uFSD
 *  (c) osFree project,
 *  author valerius, 2007, Oct 14
 *
 *  This script writes uFSD size into
 *  data field inside preldr0 (a word with
 *  offset +2 from its beginning) and then
 *  concatenates preldr0 with uFSD.
 */

parse arg args

if words(args) \= 3 then
  call usage

parse var args preldr0 ufsd ofile

preldr0_size = stream(preldr0, 'c', 'query size')
buf = charin(preldr0, 1, preldr0_size)
call charout ofile, buf

ufsd_size = stream(ufsd, 'c', 'query size')

buf = charin(ufsd, 1, ufsd_size)
call charout ofile, buf, preldr0_size + 1

buf = x2c(reverse(d2x(ufsd_size)))
call charout ofile, buf, 3


exit 0
/* --------------------------------------- */
reverse: procedure
n = arg(1)

/*
 *   Reverse byte order for little endian
 */
 l = length(n)

 if l // 2 == 1 then do
   n = '0' || n
   l = l + 1
 end

 m = l / 2

 q = ''
 do p = 0 to m - 1
   s = substr(n, 2*p + 1, 2)
   q = s || q
 end


return q
/* --------------------------------------- */
usage:

say 'Usage:'
say
say 'mkboot <preldr0> <uFSD> <outfile>'

exit -1
/* --------------------------------------- */
