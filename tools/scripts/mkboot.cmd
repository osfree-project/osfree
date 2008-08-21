/*
 *  Make boot block from bootsector, preldr0 and uFSD
 *  (c) osFree project,
 *  author valerius, 2007, Oct 14
 *
 *  This script writes uFSD and preldr0 sizes into
 *  data fields inside preldr0 (a word with offset
 *  +2 from its beginning) and then concatenates
 *  bootsector with preldr0 and uFSD.
 */

parse arg args

if words(args) \= 4 then
  call usage

parse var args bootsec preldr0 ufsd ofile

bootsec_size = stream(bootsec, 'c', 'query size')
preldr0_size = stream(preldr0, 'c', 'query size')
ufsd_size    = stream(ufsd,    'c', 'query size')

buf = charin(bootsec, 1, bootsec_size)
call charout ofile, buf

buf = charin(preldr0, 1, preldr0_size)
call charout ofile, buf, bootsec_size + 1

buf = charin(ufsd, 1, ufsd_size)
call charout ofile, buf, preldr0_size + bootsec_size + 1

buf = x2c(rev(d2x(ufsd_size)))
call charout ofile, buf, bootsec_size + 3

buf = x2c(rev(d2x(preldr0_size)))
call charout ofile, buf, bootsec_size + 5


exit 0
/* --------------------------------------- */
rev: procedure
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
say 'mkboot <bootsector> <preldr0> <uFSD> <outfile>'

exit -1
/* --------------------------------------- */
