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

parse arg args

if words(args) \= 3 then
  signal usage

parse var args file1 file2 shift

shift = strip(shift)
p = pos('0x', shift)
if p = 1 then shift = x2d(delstr(shift, p, 2))

if shift = 0 then do
  call charout stderr, 'Error: shift = 0!' || '0d 0a'x
  exit -1
end

call stream file1, 'c', 'open read'
call stream file2, 'c', 'open read'

size1 = stream(file1, 'c', 'query size')
size2 = stream(file2, 'c', 'query size')

if size1 \= size2 then do
  call charout stderr, 'Error: sizes of two files don''t equal!'
  signal quit
end

buf1 = charin(file1, 1, size1)
buf2 = charin(file2, 1, size2)

p = 1
outs = ''

do forever
  if p > size1 then leave
  u = x2d(strip(reverse(c2x(substr(buf1, p, 4))), 'L', '0'))
  v = x2d(strip(reverse(c2x(substr(buf2, p, 4))), 'L', '0'))
  if u = v then do
    p = p + 4
    iterate
  end
  w = shift / (v - u)
  /* if shift is a multiple of a difference value */
  if datatype(w, whole) then do
    b = x2b(d2x(w))
    b = strip(b, 'L', '0')
    x = lastpos('1', b)
    if x \= 1 then do
      call charout stderr, 'Error: difference is not power of 2!, x = ' || x || '0d 0a'x
      exit -1
    end
    /* the binary logarithm of w */
    n = d2x(length(b) - 1)
    if length(n) = 1 then n = '0' || n
    /* relocation item: two bytes of offset
     * followed by the log2() from shift value
     * (one byte)
     */
    rel = x2c(reverse(pad(p - 1))) || d2c(n)
    outs = outs || rel
    p = p + 4
    iterate
  end
  p = p + 1
end

/* relocation table length    */
l = length(outs)

/* Check if other bytes are
   equal in both files on the
   same positions             */
p = 1
do i = 1 to l / 2
  /* the next relocation item  */
  q = x2d(reverse(c2x(substr(outs, 3*i - 2, 2)))) + 1
  /* check all bytes before it */
  if p < q then
    do while p < q
      u = substr(buf1, p, 1)
      v = substr(buf2, p, 1)
      if u \= v then do
        call charout stderr, 'bytes not equal, pos: ' || p || '0d 0a'x
        exit -1
      end
      p = p + 1
    end
  p = q + 4
end

l = x2c(reverse(pad(l)))
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
reverse: procedure
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
