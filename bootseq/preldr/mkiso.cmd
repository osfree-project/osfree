@echo off

rem
rem make bootable ISO image
rem with FreeLdr
rem

if   not exist preldr0 wmake preldr0
if   not exist iso9660.fsd wmake iso9660.fsd
cd   ..\bootsec\eltorito
if   not exist eltorito.bin wmake eltorito.bin
cd   ..\..\preldr
copy ..\bootsec\eltorito\eltorito.bin .
call mkboot.cmd eltorito.bin preldr0 iso9660.fsd bootblock
move bootblock ..\..\..\cd\boot
del  bb
cd   ..\..\..
.\mkisofs2 -b boot/bootblock -no-emul-boot -boot-load-size 4 -boot-info-table -iso-level 3 -r -J -publisher "osFree (www.osfree.org)" -o osfree.iso cd
move osfree.iso \sys\vm\vpc\bochs\img
cd   osfree\bootseq\preldr
