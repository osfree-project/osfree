@echo off
rem Open Watcom is used to build mktables.exe
rem (this is only required if the asm tables are to be changed)
wcc -ox -3 -d__MSDOS__ mktables.c
wlink system dos file mktables.obj
