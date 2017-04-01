#
# A Makefile for DEBUG
# (c) osFree project,
# author, date
#

PROJ      = debug
TRGT      = $(PROJ).com
DESC      = DOS debugger
#defines object file names in format objname.$(O)
srcfiles  = $(p)DEBUG$(e)
# defines additional options for C compiler
ADD_ASMOPT  = -dPM=0 -dNASM=0
ADD_LINKOPT = lib $(%WATCOM)$(SEP)lib286$(SEP)noemu87.lib
COM = 1

!include $(%ROOT)/mk/appsdos.mk

# echo creating debug.com
# rem nasmw debug.asm -D PM=0 -D NASM=1 -O 2 -o debug.com -l debug.lst 
# yasm debug.asm -D PM=0 -D NASM=0 -o debug.com -l debug.lst 
#
# echo creating debugx.com
# rem nasmw debug.asm -D PM=1 -D NASM=1 -O 2 -o debugx.com -l debugx.lst 
# yasm debug.asm -D PM=1 -D NASM=0 -o debugx.com -l debugx.lst 
