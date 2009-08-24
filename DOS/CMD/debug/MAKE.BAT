@echo off

rem nasm or yasm may be used

echo creating debug.com
rem nasmw debug.asm -D PM=0 -D NASM=1 -O 2 -o debug.com -l debug.lst 
yasm debug.asm -D PM=0 -D NASM=0 -o debug.com -l debug.lst 

echo creating debugx.com
rem nasmw debug.asm -D PM=1 -D NASM=1 -O 2 -o debugx.com -l debugx.lst 
yasm debug.asm -D PM=1 -D NASM=0 -o debugx.com -l debugx.lst 
