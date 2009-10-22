/*  Generate relocation info for raw binary 32-bit executable
 *  by comparing two its versions, shifted one from another
 *  by 'shift' bytes.
 *
 *  (c) osFree project, 2007 Dec 04,
 *  author valerius (_valerius (dog) mail (dot) ru).
 *
 *  Note: file1 must be linked at base BASE,
 *  and file2 -- at base BASE + shift. Sizes of both files
 *  must be equal and <= 64k, so offsets can be 16-bits.
 *
 *  Relocation info begins from 16-bit field of its size
 *  in bytes and is followed by the array of 3-byte entries
 *  consisting of 16-bits field of offset of relocated
 *  item and 1-byte field of binary logarithm value for
 *  relocation 'shift'.
 */

numeric digits 12

crlf = '0d 0a'x

parse arg args

if words(args) \= 3 then
  signal usage

parse var args file1 file2 shift

shift = strip(shift)
p = pos('0x', shift)
if p = 1 then shift = x2d(delstr(shift, p, 2))

if shift = 0 then do
  call charout stderr, 'Error: shift = 0!' || crlf
  signal quit
end

call stream file1, 'c', 'open read'
call stream file2, 'c', 'open read'

size1 = stream(file1, 'c', 'query size')
size2 = stream(file2, 'c', 'query size')

if size1 \= size2 then do
  call charout stderr, 'Error: sizes of two files aren''t equal!' || crlf
  signal quit
end

buf1 = charin(file1, 1, size1)
buf2 = charin(file2, 1, size2)

p = 1
outs = ''

do while p < size1 - 2
  u = x2d(strip(rev(c2x(substr(buf1, p, 4))), 'L', '0'))
  v = x2d(strip(rev(c2x(substr(buf2, p, 4))), 'L', '0'))
  if u = v then do
    p = p + 1
    iterate
  end
  /* if values differs by shift */
  if shift = v - u then do
    /* ordinary 32-bit relocation */
    rel = x2c(rev(pad(p - 1))) || '00'x
    outs = outs || rel
    p = p + 4
    iterate
  end
  u = x2d(strip(rev(c2x(substr(buf1, p, 2))), 'L', '0'))
  v = x2d(strip(rev(c2x(substr(buf2, p, 2))), 'L', '0'))
  if u = v then do
    p = p + 1
    iterate
  end
  if shift / 16 = v - u then do
    rel = x2c(rev(pad(p - 1))) || '04'x
    outs = outs || rel
    p = p + 2
    iterate
  end
  p = p + 1
end

/* relocation table length    */
l = length(outs)

/* Check if remaining bytes are
   equal in both files at the
   same positions             */
p = 1
do i = 1 to l / 3
  /* the next relocation item  */
  q = x2d(rev(c2x(substr(outs, 3*i - 2, 2)))) + 1
  /* check all bytes before it */
  do while p < q
    u = substr(buf1, p, 1)
    v = substr(buf2, p, 1)
    if u \= v then do
      call charout stderr, 'bytes not equal at pos: ' || p || crlf
      signal quit
    end
    p = p + 1
  end
  p = q + 4
end

l = x2c(rev(pad(l)))
outs = l || outs

call charout ,outs

quit:

call stream file2, 'c', 'close'
call stream file1, 'c', 'close'


exit 0
/* ================================================= */
pad: procedure
s = arg(1)
/*  Convert decimal value to hexadecimal,
 *  padded to 4 hex digits
 */

s = d2x(s)
do while length(s) < 4
  s = '0' || s
end


return s
/* ================================================= */
rev: procedure
n = arg(1)

/*  Reverse byte order for little endian
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
/* ================================================= */
usage:

parse source . . name
p = lastpos('\', name)
if p > 0 then name = substr(name, p + 1)
p = lastpos('.', name)
if p > 0 then name = delstr(name, p)

say 'Generate relocation info'
say
say 'Usage:'
say
say name' file1 file2 shift >outfile.rel'
say
say 'file1 must be linked at base, less by shift'
say 'bytes than the base of file2'
say

exit 1
/* ================================================= */
