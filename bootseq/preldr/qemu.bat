@echo off
cd \qemuw32
qemu.exe -L . -cdrom \data\vm\bochs\img\%1 -m 64
cd \var\src\os2\svn\osfree\bootseq\preldr
