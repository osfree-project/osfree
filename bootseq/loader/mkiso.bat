@echo off

rem
rem make bootable ISO image
rem with FreeLdr
rem

cd ..\..\bin\boot\loader
%rexx% mkboot.cmd ..\sectors\eltorito.bin preldr0.mdl fsd\iso9660.mdl bootblk

@for %%i in (cd cd\boot cd\boot\loader cd\boot\loader\fsd cd\boot\loader\term) do @if not exist ..\..\..\..\..\%%i mkdir ..\..\..\..\..\%%i

move bootblk ..\..\..\..\..\cd\boot\
@for %%i in (preldr0.mdl preldr.rel preldr_mini.mdl freeldr.mdl chain.mdl linux.mdl bootos2.mdl *.ini *.cfg) do @copy %%i ..\..\..\..\..\cd\boot\loader\
cd fsd
copy *.mdl   ..\..\..\..\..\..\cd\boot\loader\fsd\
copy *.rel   ..\..\..\..\..\..\cd\boot\loader\fsd\
cd ..\term
copy *.mdl   ..\..\..\..\..\..\cd\boot\loader\term\
copy *.rel   ..\..\..\..\..\..\cd\boot\loader\term\

if exist %imgdir%\osfree.iso del %imgdir%\osfree.iso
cd ..\..\..\..\..\..
mkisofs  -b boot/bootblk -c boot/bootcat.bin -no-emul-boot -boot-load-size 4 -boot-info-table -iso-level 3 -allow-lowercase -no-iso-translate -r -J -U -D -publisher "osFree (www.osfree.org)" -o %imgdir%\osfree.iso cd
cd   osfree\trunk\bootseq\loader
