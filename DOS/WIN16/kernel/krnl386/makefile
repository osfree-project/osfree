#
# A Makefile for KRNL386.EXE
# (c) osFree project,
#



PROJ = krnl386
TRGT = $(PROJ).exe
DESC = Windows/386 Kernel
srcfiles = $(p)dpmildr$(e) $(p)hmemset$(e) $(p)lstrcat$(e) $(p)lstrcpy$(e) $(p)lstrlen$(e) $(p)profstrg$(e) $(p)kernel16$(e)

ADD_ASMOPT = -D?32BIT=0 -D?DEBUG=0 -D?REAL=0
JWASM = 1


!include $(%ROOT)/mk/appsdos.mk

.asm: $(MYDIR)..


