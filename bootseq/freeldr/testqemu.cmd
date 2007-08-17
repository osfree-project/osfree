@echo off
rem
rem Launch qemu with disk image
rem

cd \sys\vm\vpc\bochs\img
qemu -hda os3-ext2-lilo.img >\dev\nul 2>&1
cd \var\src\os2\boot\freeldr
