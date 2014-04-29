@echo off
%1
cd\
md fat32temp
copy %1\fat32\os2fat32.zip %1\fat32temp
cd\fat32temp
%1\tools\unzip os2fat32
%1\os2\xcopy fat32.ifs %1\os2 /o
%1\os2\xcopy cachef32.exe %1\os2 /o
%1\os2\xcopy f32stat.exe %1\os2 /o
%1\os2\xcopy monitor.exe %1\os2 /o
%1\os2\xcopy ufat32.dll %1\os2\dll
%1\os2\xcopy partfilt.flt %1\os2\boot
del %1\fat32\*.* /n
%1\os2\xcopy *.* %1\fat32
del %1\fat32temp\*.* /n
cd\
rd fat32temp
del %1\fat32\os2fat32.zip /n
