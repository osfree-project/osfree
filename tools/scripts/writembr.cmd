/*
 *   A script to write mbr boot code into HDD image.
 *   (c) osFree project,
 *   valerius 2006/10/09
 */

 parse arg diskimage mbr

 /* MBR signature (0x55aa) offset */
 SignatureOfs = 512 - 2
 /* PT size                       */
 PTSize = 16 * 4
 /* MBR code size == PT offset    */
 CodeSize     = 512 - PTSize - 2

 if diskimage = '' | mbr = '' then do
   call usage
   exit -1
 end

 rc = stream(mbr, 'c', 'query exist')

 if rc = '' then do
   say mbr' doesn''t exist!'
   exit -1
 end

 rc = stream(mbr, 'c', 'open read')

 if rc \= 'READY:' then do
   say 'Can''t open 'mbr' for reading!'
   exit -1
 end

 buf = charin(mbr, 1, CodeSize)

 rc = stream(diskimage, 'c', 'open write')

 if rc \= 'READY:' then do
   say 'Can''t open 'diskimage' for writing!'
   exit - 1
 end

 call charout diskimage, buf, 1

 buf = charin(mbr, SignatureOfs + 1, 2)
 call charout diskimage, buf, SignatureOfs + 1

 call stream diskimage, 'c', 'close'
 call stream mbr, 'c', 'close'


 exit 0
/* --------------------------------------------- */
usage:

 say 'Usage:'
 say
 say 'writembr <diskimage> <mbr>'
 say


return
/* --------------------------------------------- */
