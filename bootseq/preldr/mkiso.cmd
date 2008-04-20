@echo off

rem .......................... 
rem (c) osFree project, 2008 .
rem make bootable ISO image  .
rem with FreeLdr             .
rem ..........................

@echo Creating bootable iso image...

rem Current working directory
set cwd=%ROOT%\bootseq\preldr

set dir1=%root%\bootseq\preldr
set files1=preldr0 iso9660.fsd

set dir2=%root%\bootseq\bootsec\eltorito
set files2=eltorito.bin

@for %%l in (1 2) do ^
  (set f=%%files%%l%% && ^
   set d=%%dir%%l%%   && ^
   (@for %%i in (%f%) do ^
     (cd %d% && (@wmake %%i) && cd %cwd%)))

@call mkboot.cmd ..\bootsec\eltorito\eltorito.bin preldr0 iso9660.fsd bootblk

cd ..\..\..
set dirs=cd cd\boot cd\boot\freeldr cd\boot\freeldr\fsd ^
         cd\boot\freeldr\term cd\l4 cd\pns cd\os2 cd\l4ka
@for %%i in (%dirs%) do if not exist %%i mkdir %%i
cd osfree\bootseq\preldr

@move bootblk ..\..\..\cd\boot
set files=preldr0 preldr.ini freeldr freeldr.cfg boot_lin ^
          boot_chn boot_dsk boot
@for %%i in (%files%) do if exist %%i copy %%i  ..\..\..\cd\boot\freeldr

@copy *.fsd   ..\..\..\cd\boot\freeldr\fsd
@copy *.trm   ..\..\..\cd\boot\freeldr\term
@copy *.rel   ..\..\..\cd\boot\freeldr\fsd

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
@for %%i in (%files%) do if exist %%i copy %%i ..\..\..\cd\os2\
cd   ..\..\..

@.\mkisofs2 -b boot/bootblk -c boot/bootcat.bin -no-emul-boot -boot-load-size 12 -boot-info-table -iso-level 3 -allow-lowercase -no-iso-translate -r -J -publisher "osFree (www.osfree.org)" -o osfree.iso cd
@del %imgdir%\osfree.iso
@move osfree.iso %imgdir%
cd %root%\bootseq\preldr
