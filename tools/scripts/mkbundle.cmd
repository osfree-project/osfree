/* Make a bundle from the startup, microfsd, minifsd and os2ldr.
 *
 * syntax: mkbundle <startup> <microfsd> <minifsd> <os2ldr> <outfile>
 */

parse arg startup ufsd mfsd ldr outfile

call stream startup, 'c', 'open read'
call stream ufsd,    'c', 'open read'
call stream mfsd,    'c', 'open read'
call stream ldr,     'c', 'open read'

call stream outfile, 'c', 'open'

startup_sz = stream(startup, 'c', 'query size')
ufsd_sz    = stream(ufsd,    'c', 'query size')
mfsd_sz    = stream(mfsd,    'c', 'query size')
ldr_sz     = stream(ldr,     'c', 'query size')

buf = charin(startup, 1, startup_sz)
call charout outfile, buf, 1

buf = charin(ufsd, 1, ufsd_sz)
call charout outfile, buf, 1 + startup_sz

buf = charin(mfsd, 1, mfsd_sz)
call charout outfile, buf, 1 + startup_sz + ufsd_sz

buf = charin(ldr, 1, ldr_sz)
call charout outfile, buf, 1 + startup_sz + ufsd_sz + mfsd_sz

buf = pad(x2c(rev(d2x(startup_sz))))
call charout outfile, buf, 32 + 1

buf = pad(x2c(rev(d2x(ufsd_sz))))
call charout outfile, buf

buf = pad(x2c(rev(d2x(mfsd_sz))))
call charout outfile, buf

buf = pad(x2c(rev(d2x(ldr_sz))))
call charout outfile, buf

call stream outfile, 'c', 'close'

/* fix multiboot header to add ufsd_sz + mfsd_sz + ldr_sz
   to exe_end and bss_end */
buf = charin(startup, x2d('80') + 20 + 1, 4)
buf = pad(x2c(rev(d2x(x2d(rev(c2x(buf))) + ufsd_sz + mfsd_sz + ldr_sz))))
call charout outfile, buf, x2d('80') + 20 + 1

buf = charin(startup, x2d('80') + 24 + 1, 4)
buf = pad(x2c(rev(d2x(x2d(rev(c2x(buf))) + ufsd_sz + mfsd_sz + ldr_sz))))
call charout outfile, buf, x2d('80') + 24 + 1

call stream outfile, 'c', 'close'

call stream ldr,     'c', 'close'
call stream mfsd,    'c', 'close'
call stream ufsd,    'c', 'close'
call stream startup, 'c', 'close'


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
pad: procedure
s = arg(1)

do while length(s) < 4
  s = s || x2c('00')
end


return s
/* --------------------------------------- */
