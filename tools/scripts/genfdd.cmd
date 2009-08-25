/*
 *  Generate floppy image with 
 *  ext2fs filesystem on it.
 *  (c) osFree project,
 *  valerius, 2006/11/07
 */

parse arg args

/* Determine system type */
parse source sys .

/* System-dependent variables */
if sys = 'OS/2'    | sys = 'DOS'     |,
   sys = 'WINDOWS' | sys = 'Windows' |,
   sys = 'WINNT'   | sys = 'NT'   then do
  drop sys
  opt.sys = 'OS/2'
end; else
if sys = 'UNIX'    | sys = 'Linux'   |,
   sys = 'LINUX'   then do
  drop sys
  opt.sys = 'UNIX'
end

/* Initialize common variables */
opt.cyl        = 80
opt.heads      = 2
opt.sectors    = 18
opt.bs         = 'bootsect_1.bin'
opt.bpbcfg     = 'diskette.cfg'
opt.Image      = ''
opt.outImage   = ''
opt.findfile   = ''
/* Size of current partition in sectors */
opt.NumSectors = 0

call parse_cmd_line

opt.outImage = opt.Image

if opt.Image = '' then do
  call give_help
  exit -1
end

call add_bootsector


exit 0
/* ====================================================== */
add_bootsector: procedure expose opt.

  partfile = opt.Image
  bsfile   = opt.bs
  offset   = 0

  rc = stream(partfile, 'c', 'open')

  rc  = stream(bsfile,    'c', 'open')
  buf = charin(bsfile,, 3)

  /* Create BPB structure     */
  bpb = create_bpb()
  buf = buf || bpb
  /* End create BPB structure */

  skip_size = length(bpb) + 3

  count = 512 - skip_size - 2 - 5
  /* Copy the bootloader      */
  buf = buf || charin(bsfile, skip_size + 1, count)

  rc = stream(bsfile, 'c', 'close')

  /* Call findfile.cmd to determine muFSD offset
     and length inside the partition image       */
  parse value findfile(opt.findfile) with mu_offset mu_size

  say 'muFSD offset: 0x'mu_offset
  say 'muFSD size:   0x'mu_size

  buf = buf ||,
        x2c(reverse(pad(mu_size  , 1))) ||,  /* muFSD length            */
        x2c(reverse(pad(mu_offset, 4))) ||,  /* muFSD 1st sector number */
        '55 AA'x                             /* Signature               */

  /* Write bootsector into HDD image */
  call charout partfile, buf, offset + 1

  rc = stream(opt.outImage, 'c', 'close')


return
/* ====================================================== */
create_bpb: procedure expose opt.

tracksize = opt.sectors
heads     = opt.heads

ret = stream(opt.bpbcfg, 'c', 'query exists')
if ret = '' then do
  say 'File ' || opt.bpbcfg || ' doesn''t exist!'
  exit -1
end

rc = stream(opt.bpbcfg, 'c', 'open read')

do while lines(opt.bpbcfg)
  line = linein(opt.bpbcfg)
  p = pos('#', line)
  if p > 0 then line = delstr(line, p)
  if line = '' then iterate
  interpret(line)
end

rc = stream(opt.bpbcfg, 'c', 'close')

nsecs = opt.NumSectors
if nsecs >= 65535 then do
  nsecs_ext = nsecs
  nsecs = 0
end

bpb        = ''
bpb        = bpb   ||,
             oemid ||,
             x2c(reverse(pad(d2x(sectorsize), 2)))  ||,
             x2c(reverse(pad(d2x(clustersize), 1))) ||,
             x2c(reverse(pad(d2x(res_sectors), 2))) ||,
             x2c(reverse(pad(d2x(nfats), 1)))       ||,
             x2c(reverse(pad(d2x(rootdirsize), 2))) ||,
             x2c(reverse(pad(d2x(nsecs), 2)))       ||,
             mediadesc                              ||,
             x2c(reverse(pad(d2x(fatsize), 2)))     ||,
             x2c(reverse(pad(d2x(tracksize), 2)))   ||,
             x2c(reverse(pad(d2x(headscount), 2)))  ||,
             x2c(reverse(pad(d2x(hiddensecs), 4)))  ||,
             x2c(reverse(pad(d2x(nsecs_ext), 4)))   ||,
             disknum  || logdrive || marker         ||,
             volserno || vollabel || filesys


return bpb
/* ====================================================== */
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
/* ====================================================== */
lshift: procedure
/*
 *   Shift a (decimal) number to left by n bits
 */
s = arg(1)
n = arg(2)

p = ''

s = x2b(d2x(s))

do i = 1 to n
  s = s || '0'
end


return x2d(b2x(s))
/* ====================================================== */
pad: procedure
/*
 *   Pad a hex number by zeroes from left
 */
s = arg(1)
n = arg(2)


if n = '' then n = 2

if length(s) > 2*n then return s

do while length(s) < 2*n
  s = '0' || s
end


return s
/* ====================================================== */
parse_cmd_line: procedure expose args opt.
/*
 *  Parse a command line
 */

l = 0
m = 0

do while args \= ''
  parse value getvar() with opt ':' val
  select
    when opt = 'b'   then do
      opt.bs       = val
      iterate
    end
    when opt = 'B'   then do
      opt.bpbcfg   = val
      iterate
    end
    when opt = 'f'   then do
      s = val
      s = strip(s)
      s = strip(s, 'B', '"')
      s = strip(s)
      opt.findfile = s
      iterate
    end
    when opt = 'H'   then do
      opt.heads    = val
      iterate
    end
    when opt = 'S'   then do
      opt.sectors  = val
      iterate
    end
    when opt = 'i'   then do
      opt.Image = val
      iterate
    end
    when opt = 'h'   then do
      call give_help
      exit 0
    end
    otherwise do
      say 'opt = "'opt'"'
      /* call give_help */
      exit -1
    end
  end
end


return
/* ====================================================== */
getvar: procedure expose args

opt1  = getarg()
opt1  = delstr(opt1, 1, 1)

args = strip(args)
if pos('-', args) = 1 then
  opt2 = ''
else
  opt2 = getarg()


return opt1':'opt2
/* ====================================================== */
getarg: procedure expose args

/* Gets one word, or a line, enclosed
   in quotes, from args               */

args = strip(args)

if pos('"', args) == 1 then
  parse value args with '"' opt '"' args
else
  parse var args opt args



return opt
/* ====================================================== */
give_help:

say
say 'Add the bootsector to floppy image'
say '(c) osFree project,'
say 'author Valery V. Sedletski,'
say 'aka valerius'
say ''
say 'Syntax:'
say 'genfdd <options> -i <input image> ...'
say 'where options are:'
say ''
say '-i:   an input image of a floppy disk'
say '-b:   a file with bootsector (default is `bootsect_1.bin'')'
say '-f:   command line for findfile.cmd'
say '-S:   sector per track count'
say '-B:   bpb.cfg name'
say '-H:   heads count for the HDD image'
say '      input and output images are mandatory.'
say '-h:   give help (this screen)'
say


return
/* ====================================================== */
