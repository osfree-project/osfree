#
# A Makefile for KRNL286.EXE
# (c) osFree project,
#



PROJ = krnl286
TRGT = $(PROJ).exe
DESC = Windows/286 Kernel
srcfiles = $(p)dpmildr$(e) $(p)hmemset$(e) $(p)lstrcat$(e) $(p)lstrcpy$(e) $(p)lstrlen$(e) $(p)profstrg$(e) $(p)kernel16$(e)

ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=0 -D?REAL=0
JWASM = 1
DEST        = os2$(SEP)mdos$(SEP)winos2$(SEP)system

!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR)..


