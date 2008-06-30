rem @echo off

rem ............................
rem . (c) osFree project, 2008 .
rem . make bootable ISO image  .
rem . with FreeLdr             .
rem ............................

@echo Creating bootable iso image...

rem Current working directory
set cwd=%ROOT%\bootseq\preldr

rem ---------prereqs-------------------
set dir1=%cwd%
set files1=preldr0 iso9660.fsd

set dir2=%root%\bootseq\bootsec\eltorito
set files2=eltorito.bin
rem ---------prereqs-------------------

@for %%l in (1 2) do ^
 (set f=%%files%%l%% && ^
  set d=%%dir%%l%% && (@for %%i in (%f%) do ^
  (cd %d% && (@if not exist %%i @wmake %%i))))

cd %cwd%
@%rexx% mkboot.cmd ..\bootsec\eltorito\eltorito.bin preldr0 iso9660.fsd bootblk

cd ..\..\..
set dirs=cd cd\boot cd\boot\freeldr cd\boot\freeldr\fsd ^
         cd\boot\freeldr\term cd\l4 cd\pns cd\os3 cd\l4ka
@for %%i in (%dirs%) do if not exist %%i mkdir %%i

cd osfree\bootseq\preldr
@move bootblk ..\..\..\cd\boot
set files=preldr0 preldr.ini freeldr freeldr.cfg bt_linux           bt_chain bt_disk bt_os2 bt_bsd
@for %%i in (%files%) do if exist %%i copy %%i  ..\..\..\cd\boot\freeldr

@copy *.fsd ..\..\..\cd\boot\freeldr\fsd
@copy *.trm ..\..\..\cd\boot\freeldr\term
@copy *.rel ..\..\..\cd\boot\freeldr\fsd

@move ..\..\..\cd\boot\freeldr\fsd\preldr0.rel ..\..\..\cd\boot\freeldr
set files=serial.rel hercules.rel console.rel
for %%i in (%files%) do if exist %%i move ..\..\..\cd\boot\freeldr\fsd\%%i ..\..\..\cd\boot\freeldr\term

cd ..\..\fiasco
@copy * ..\..\cd\l4
cd ..\pns
@copy * ..\..\cd\pns
cd ..\files
@copy * ..\..\cd\l4ka

cd ..\os2\server
set files=os2server VioWrtTTY_test config.sys libkal.s.so os2.cfg MiniELFExe.Exe
@for %%i in (%files%) do if exist %%i copy %%i ..\..\..\cd\os3\
cd   ..\..\..

@if not exist files goto mkiso

rem Optional files

set dirs=bootimgs floppies linux memdisk memtest86

for %i in (%dirs%) do ^
  (@if exist files\%i (if not exist cd\%i ^
     mkdir cd\%i) && ^
     copy files\%i\* cd\%i)

set files=os2ldr os2ldr.msg os2ldr.ini os2boot os2dump os2krnl config.sys
@for %i in (%files%) do @if exist files\%i @copy files\%i cd

:mkiso

@del %imgdir%\osfree.iso
@%mkisofs% -b boot/bootblk -c boot/bootcat.bin -no-emul-boot -boot-load-size 12 -boot-info-table -iso-level 3 -allow-lowercase -no-iso-translate -r -J -publisher "osFree (www.osfree.org)" -o %imgdir1%/osfree.iso cd
rem -eltorito-alt-boot -b floppies/os2boot.img
cd %cwd%
