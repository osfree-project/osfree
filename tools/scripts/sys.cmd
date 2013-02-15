/*
 *   sys.cmd:
 *   A REXX script to write a bootsector image
 *   to a disk image file and make it bootable.
 *   (c) osFree project,
 *   valerius 2006/10/04.
 */

 parse arg diskimage bsfile hdd

/*
 *  for the file os2boot on the disk image:
 *  21h -- starting sector;
 *  3h -- length
 */

 LengthOffset = 512 - 3
 AddrOffset   = 512 - 7

 Length = '1E'x /* '03'x */            /* 3 sectors long */
 Addr   = '41 BA'x /* '03 94 79'x */ /* '00 00 01 59' */    /* '00 06 7A F6'x */  /* 0x21 for floppy,'01 07'x 0x198 - 0x3f = 0x159 for hdd, 0x67AF6 for hdd1 */

 if diskimage = '' | bsfile = '' then do
   call usage
   exit - 1
 end
say '' diskimage, 'c', 'query exists'
 if stream(diskimage, 'c', 'query exists') = '' then do
   say diskimage' doesn''t exist!'
   exit -1
 end

 if stream(bsfile, 'c', 'query exists') = '' then do
   say bsfile' doesn''t exist!'
   exit -1
 end

 rc = stream(bsfile, 'c', 'open read')

 if rc \= 'READY:' then do
   say 'Can''t open file 'bsfile'!'
   exit -1
 end

 bs_size = stream(bsfile, 'c', 'query size')

 if hdd = 'hdd'
   then bootsector = x2d('03 52 BF') * 512  /* x2d('06 79 EF') * 512; 63 * 512 */
   else bootsector = 0


 /* Read jmp instruction and SysId */
 buf = charin(bsfile, 1, 11)

 rc = stream(diskimage, 'c', 'open')

 if rc \= 'READY:' then do
   say 'Can''t open file 'diskimage'!'
   exit -1
 end

 call charout diskimage, buf, bootsector + 1

 /* Read jump address and calculate end of BPB position      */
 buf = charin(bsfile, 2, 1)
 buf = c2d(buf)
 start = buf + 2 - 4 * 2

 /* Count of bytes to read at the end of bootsector
    at start                                                 */
 count = 512 - start

 buf = charin(bsfile, start + 1, count)
 call charout diskimage, buf, bootsector + start + 1

 /* Write an address and length of the file to load
    by bootsector (os2ldr.sym at the beginning of data area) */
 call charout diskimage, reverse(Length), bootsector + LengthOffset
 call charout diskimage, reverse(Addr), bootsector + AddrOffset

 call InitBPB

 fatStart = bpb.resSectors * bpb.secSize
 size     = bpb.fatSize * bpb.secSize
 fat      = charin(diskimage, fatStart + 1, size)

/* say '"'bpb.fsName'"' */

 call stream diskimage, 'c', 'close'
 call stream bsfile,    'c', 'close'


 exit 0
/* ------------------------------------------------ */
InitBPB: procedure expose diskimage bootsector fields sizes bpb.

 fields =  'SECSIZE CLUSIZE RESSECTORS NFATS ROOTDIRSIZE NSEC',
           'MEDIADESC FATSIZE TRACKSIZE HEADSCNT HIDDENSECS',
           'NSECTEXT DISKNUM LOGDRIVE MARKER VOLSERNO VOLLABEL FSNAME'

 sizes  =  '2 1 2 1 2 2  1 2 2 2 4 4  1 1  1 4 11 8'

 offset = bootsector + 11

 n = words(fields)
 do i = 1 to n
   name = word(fields, i)
   size = word(sizes, i)

   field = charin(diskimage, offset + 1, size)
   if i < n - 1 then
     field = reverse(c2x(field))
   if i < n - 2 then
     field = x2d(field)
   bpb.name = field
   offset = offset + size
 end


return
/* ------------------------------------------------ */
reverse: procedure
n = arg(1)

/*
 *   Reverse byte order for little endian
 */

 m = length(n) / 2

 q = ''
 do p = 0 to m - 1
   s = substr(n, 2*p + 1, 2)
   q = s || q
 end


return q
/* ------------------------------------------------ */
usage:

 say 'Usage:'
 say
 say 'sys <diskimage> <bootsector>'
 say


return
/* ------------------------------------------------ */
