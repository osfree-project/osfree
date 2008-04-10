rem @echo off

rem
rem make bootable ISO image
rem with FreeLdr
rem

set rexx=c:\regina\rexx.exe
set path=.\cdrtoolsw32;%path%

if not exist preldr0 wmake preldr0
if not exist iso9660.fsd wmake iso9660.fsd
cd ..\bootsec\eltorito
if not exist eltorito.bin wmake eltorito.bin
cd ..\..\preldr
copy ..\bootsec\eltorito\eltorito.bin .
%rexx% mkboot.cmd eltorito.bin preldr0 iso9660.fsd bootblk

if not exist ..\..\..\cd mkdir ..\..\..\cd
if not exist ..\..\..\cd\boot mkdir ..\..\..\cd\boot
if not exist ..\..\..\cd\boot\freeldr mkdir ..\..\..\cd\boot\freeldr
if not exist ..\..\..\cd\boot\freeldr\fsd mkdir ..\..\..\cd\boot\freeldr\fsd
if not exist ..\..\..\cd\l4 mkdir ..\..\..\cd\l4
if not exist ..\..\..\cd\pns mkdir ..\..\..\cd\pns
if not exist ..\..\..\cd\os2 mkdir ..\..\..\cd\os2

move bootblk ..\..\..\cd\boot
copy preldr0 ..\..\..\cd\boot\freeldr
copy preldr.ini  ..\..\..\cd\boot\freeldr
copy freeldr.cfg ..\..\..\cd\boot\freeldr
copy *.fsd   ..\..\..\cd\boot\freeldr\fsd
copy *.rel   ..\..\..\cd\boot\freeldr\fsd
move ..\..\..\cd\boot\freeldr\fsd\preldr0.rel ..\..\..\cd\boot\freeldr\
copy freeldr    ..\..\..\cd\boot\freeldr
copy boot_linux ..\..\..\cd\boot\freeldr
copy boot_chain ..\..\..\cd\boot\freeldr

cd ..\..\fiasco
copy * ..\..\cd\l4
cd ..\pns
copy * ..\..\cd\pns
cd ..\os2\server
copy os2server ..\..\..\cd\os2
copy VioWrtTTY_test ..\..\..\cd\os2
copy config.sys ..\..\..\cd
copy libkal.s.so ..\..\..\cd\os2
copy os2.cfg ..\..\..\cd\os2
copy MiniELFExe.Exe ..\..\..\cd\os2
cd   ..\..\..

osfree\bootseq\preldr\cdrtoolsw32\mkisofs  -b boot/bootblk -c boot/bootcat.bin -no-emul-boot -boot-load-size 8 -boot-info-table -iso-level 3 -allow-lowercase -no-iso-translate -r -J -publisher "osFree (www.osfree.org)" -o osfree.iso cd
move osfree.iso \data\vm\bochs\img
cd   osfree\bootseq\preldr
