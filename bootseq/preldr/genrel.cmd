/*  Generate relocation info for raw binary 32-bit executable
 *  by comparing two its versions, shifted one from another
 *  by 0x10000 bytes.
 *
 *  (c) osFree project, 2007 Dec 04,
 *  author valerius (_valerius (dog) mail (dot) ru).
 *
 *  Note: file1 must be linked at base 0x8xxxxx,
 *  and file2 -- at base 0x7xxxxx. Size of both
 *  files must be <= 0x10000 bytes
 */

parse arg args

if words(args) \= 2 then
  signal usage

parse var args file1 file2

call stream file1, 'c', 'open read'
call stream file2, 'c', 'open read'

size1 = stream(file1, 'c', 'query size')
size2 = stream(file2, 'c', 'query size')

buf1 = charin(file1, 1, size1)
buf2 = charin(file2, 1, size2)

call stream file2, 'c', 'close'
call stream file1, 'c', 'close'

p = -2
outs = ''

do forever
  p = pos('08 00'x, buf1, p + 4)
  if p <= 0 then
    leave
  else do
    if p < 3 then iterate
    if substr(buf2, p, 2) = '07 00'x &,
       substr(buf1, p - 2, 2) = substr(buf2, p - 2, 2)
    then do
      rel = x2c(reverse(pad(p - 3)))
      outs = outs || rel
    end
  end
end

/* relocation table length */
l = x2c(reverse(pad(length(outs))))
outs = l || outs

call charout ,outs


exit 0
/* ================================================= */
pad: procedure
s = arg(1)

s = d2x(s)
do while length(s) < 4
  s = '0' || s
end


return s
/* ================================================= */
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
say name' file1 file2 >outfile.rel'
say

exit 1
/* ================================================= */
