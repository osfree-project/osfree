/*
 *   sys.cmd:
 *   A REXX script to write a bootsector image
 *   to a disk image file and make it bootable.
 *   (c) osFree project,
 *   valerius 2006/10/04.
 */

 parse arg diskimage mapfile hdd


 if diskimage = '' | mapfile = '' then do
   call usage
   exit - 1
 end

 if stream(diskimage, 'c', 'query exist') = '' then do
   say diskimage' doesn''t exist!'
   exit -1
 end

 if stream(mapfile, 'c', 'query exist') = '' then do
   say mapfile' doesn''t exist!'
   exit -1
 end

 rc = stream(mapfile, 'c', 'open read')

 if rc \= 'READY:' then do
   say 'Can''t open file 'mapfile'!'
   exit -1
 end

 if hdd = 'hdd'
   then mapsector = x2d('40') * 512
   else mapsector = 0


 /* Read the map */
 buf = charin(mapfile, 1, 512)

 rc = stream(diskimage, 'c', 'open')

 if rc \= 'READY:' then do
   say 'Can''t open file 'diskimage'!'
   exit -1
 end

 call charout diskimage, buf, mapsector + 1


 exit 0
/* ------------------------------------------------ */
usage:

 say 'Usage:'
 say
 say 'sys <diskimage> <bootsector>'
 say


return
/* ------------------------------------------------ */
