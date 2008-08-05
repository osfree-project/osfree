@echo off
cd \data\vm\img
qemu.exe -m 64 %1 %2 %3 %4 %5 %6 %7 %8 %9
cd \var\src\os2\svn\osfree\bootseq\preldr
del \tmp\qemu*
