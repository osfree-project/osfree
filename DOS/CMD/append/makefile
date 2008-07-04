# Makefile for building under Linux
#

UPX=~/fdos/upx/upx

INSTDIR=../../bin

all: append.exe

install: all
	$(UPX) --8086 -f -o $(INSTDIR)/append.exe append.exe
	
clean:
	rm -f append.exe

append.exe: append.asm cmdline.asm environ.asm int21.asm int2f.asm useful.mac exebin.mac
	nasm -dNEW_NASM -fbin append.asm -o $@

