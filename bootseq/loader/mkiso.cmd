echo off

rem ............................
rem . (c) osFree project, 2008 .
rem . make bootable ISO image  .
rem . with FreeLdr             .
rem ............................

@echo Creating bootable iso image...

rem Current working directory
set cwd=%ROOT%\bootseq\loader

rem ---------prereqs-------------------
set dir1=%cwd%\preldr
set files1=preldr0.mdl preldr0.rel mini\preldr_mini.mdl mini\preldr_mini.rel

set dir2=%cwd%\filesys
set files2=iso9660.mdl iso9660.rel

set dir3=%cwd%\..\bootsec\eltorito
set files3=eltorito.bin
rem ---------prereqs-------------------

@for %%l in (1 2 3) do ^
 (set f=%%files%%l%% && ^
  set d=%%dir%%l%% && (@for %%i in (%f%) do ^
  (cd %d% && (@if not exist %%i @wmake %%i) && cd ..)))

cd %ROOT%bin

@mkboot.cmd boot\sectors\eltorito.bin boot\loader\preldr0.mdl boot\loader\fsd\iso9660.mdl boot\bootblk
rem @mkboot.cmd boot\sectors\eltorito.bin boot\loader\preldr_mini.mdl boot\loader\fsd\iso9660.mdl boot\bootblk

cd ..\..\..
set dirs=cd cd\boot cd\boot\loader cd\boot\loader\fsd ^
         cd\boot\loader\term cd\l4 cd\pns cd\os3 cd\l4ka
@for %%i in (%dirs%) do if not exist %%i mkdir %%i

cd osfree\trunk\bin

rem @move ..\..\cd\boot\freeldr\fsd\preldr0.rel ..\..\..\cd\boot\freeldr
rem set files=serial.rel hercules.rel console.rel
rem for %%i in (%files%) do if exist %%i move ..\..\..\cd\boot\freeldr\fsd\%%i ..\..\..\cd\boot\freeldr\term

cd boot

@move bootblk ..\..\..\..\cd\boot
set files=preldr0.mdl preldr0.rel preldr_mini.mdl preldr_mini.rel preldr.ini freeldr.mdl freeldr.rel boot.cfg linux.mdl linux.rel ^
    chain.mdl chain.rel disk.mdl disk.rel bootos2.mdl bootos2.rel bsd.mdl bsd.rel
@for %%i in (%files%) do if exist loader\%%i copy loader\%%i  ..\..\..\..\cd\boot\loader

@copy loader\fsd\* ..\..\..\..\cd\boot\loader\fsd
@copy loader\term\* ..\..\..\..\cd\boot\loader\term

cd ..\..\fiasco
@copy * ..\..\..\cd\l4
cd ..\pns
@copy * ..\..\..\cd\pns
cd ..\pistachio
@copy * ..\..\..\cd\l4ka

cd ..\os2\server
set files=os2server VioWrtTTY_test config.sys libkal.s.so os2.cfg MiniELFExe.Exe
@for %%i in (%files%) do if exist %%i copy %%i ..\..\..\cd\os3\
cd   ..\..\..\..

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
@%mkisofs% -b boot/bootblk -c boot/bootcat.bin -no-emul-boot -boot-info-table -boot-load-size 4 -iso-level 3 -relaxed-filenames -r -J -joliet-long -l -D -N -U -allow-lowercase -no-iso-translate -v -x .DS_Store -publisher "osFree (www.osfree.org)" -o %imgdir1%/osfree.iso cd
rem @%mkisofs% -b boot/isolinux/isolinux.bin -c boot/isolinux/bootcat.bin -no-emul-boot -boot-info-table -boot-load-size 4 -iso-level 3 -relaxed-filenames -r -J -joliet-long -l -D -N -U -allow-lowercase -no-iso-translate -v -x .DS_Store -publisher "osFree (www.osfree.org)" -o %imgdir1%/osfree.iso cd
rem -eltorito-alt-boot -b floppies/os2boot.img  -allow-lowercase -no-iso-translate -U -D
rem  -b boot.img -no-emul-boot -boot-load-seg 1984 -boot-load-size 4 -iso-level 2 -J -joliet-long -l -D -relaxed-filenames -N -V WinXP -v -x .DS_Store -o ../WinXP.iso .

cd %cwd%

:end